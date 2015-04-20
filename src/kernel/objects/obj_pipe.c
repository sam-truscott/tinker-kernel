/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_pipe.h"

#include "arch/tgt_types.h"
#include "arch/tgt.h"
#include "object_private.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/collections/unbounded_list.h"
#include "kernel/utils/collections/unbounded_list_iterator.h"

UNBOUNDED_LIST_TYPE(pipe_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_SPEC_ADD(static, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_BODY_CREATE(static, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_BODY_ADD(static, pipe_list_t, object_pipe_t*)

UNBOUNDED_LIST_ITERATOR_TYPE(pipe_list_it_t)
UNBOUNDED_LIST_ITERATOR_INTERNAL_TYPE(pipe_list_it_t, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_ITERATOR_SPEC(static, pipe_list_it_t, pipe_list_t, object_pipe_t*)
UNBOUNDED_LIST_ITERATOR_BODY(static, pipe_list_it_t, pipe_list_t, object_pipe_t*)

typedef struct rx_data
{
	uint32_t free_messages;
	uint32_t total_messages;
	uint32_t read_msg_position;
	uint32_t write_msg_position;
	uint32_t message_size;
	object_thread_t * blocked_owner;
	uint8_t * read_msg_ptr;
	uint8_t * write_msg_ptr;
	pipe_list_t * senders;
} rx_data_t;

typedef struct tx_data
{
	object_thread_t * sending_thread;
	pipe_list_t * readers;
} tx_data_t;

typedef struct object_pipe_t
{
	object_internal_t object;
	tinker_pipe_direction_t direction;
	mem_pool_info_t * pool;
	uint8_t * memory;
	rx_data_t rx_data;
	tx_data_t tx_data;
	char name[MAX_SHARED_OBJECT_NAME_LENGTH];
} object_pipe_internal_t;

object_pipe_t * obj_cast_pipe(object_t * const o)
{
	object_pipe_t * result = NULL;
	if(o)
	{
		const object_pipe_t * const tmp = (const object_pipe_t*)o;
		if (tmp->object.type == PIPE_OBJ)
		{
			result = (object_pipe_t*)tmp;
		}
	}
	return result;
}

object_number_t obj_pipe_get_oid
	(const object_pipe_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

static bool_t pipe_can_receive(const object_pipe_t * const pipe)
{
	return pipe
			&& ((pipe->direction == PIPE_RECEIVE || pipe->direction == PIPE_SEND_RECEIVE)
			&& pipe->rx_data.free_messages > 0);
}

static void pipe_receive_message(
		object_pipe_t * const receiver,
		const void * message,
		const uint32_t message_size)
{
#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Writing size to %x\n", receiver->rx_data.write_msg_ptr);
	debug_print("Pipe: Writing data to %x\n", receiver->rx_data.write_msg_ptr+sizeof(uint32_t));
#endif
	uint32_t * const msg_size = (uint32_t*)receiver->rx_data.write_msg_ptr;
	*msg_size = message_size;
	util_memcpy(receiver->rx_data.write_msg_ptr+sizeof(uint32_t), message, message_size);
	if ((receiver->rx_data.write_msg_position + 1) == receiver->rx_data.total_messages)
	{
		receiver->rx_data.write_msg_position = 0;
		receiver->rx_data.write_msg_ptr = receiver->memory;
	}
	else
	{
		receiver->rx_data.write_msg_position++;
		receiver->rx_data.write_msg_ptr += (receiver->rx_data.message_size+sizeof(uint32_t));
	}
#if defined(PIPE_DEBUGGING)
	const uint32_t old_free = receiver->rx_data.free_messages;
#endif
	receiver->rx_data.free_messages--;
#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Receiver has %d -> %d free messages\n", old_free, receiver->rx_data.free_messages);
#endif
	if (obj_thread_is_waiting_on(receiver->rx_data.blocked_owner, (object_t*)receiver))
	{
#if defined(PIPE_DEBUGGING)
		debug_print("Pipe: Blocked owner - resuming them\n");
#endif
		obj_set_thread_ready(receiver->rx_data.blocked_owner);
		receiver->rx_data.blocked_owner = NULL;
	}
}

static inline bool_t pipe_is_receiver(const object_pipe_t * const pipe)
{
	return ((pipe->direction == PIPE_RECEIVE) || (pipe->direction == PIPE_SEND_RECEIVE));
}

static inline bool_t pipe_is_sender(const object_pipe_t * const pipe)
{
	return ((pipe->direction == PIPE_SEND) || (pipe->direction == PIPE_SEND_RECEIVE));
}

static bool_t pipe_can_send_to_all(
		const object_pipe_t * const pipe,
		const bool_t blocking)
{
	pipe_list_it_t it;
	object_pipe_t * receiver = NULL;
	bool_t can_send_all = true;

	pipe_list_it_t_initialise(&it, pipe->tx_data.readers);
	if (pipe_list_it_t_get(&it, &receiver))
	{
		while(receiver)
		{
			const bool_t can_send_pipe = pipe_can_receive(receiver);
			if (!can_send_pipe && blocking)
			{
				pipe_list_t_add(receiver->rx_data.senders, (object_pipe_t*)pipe);
			}
			can_send_all = (!can_send_all) ? can_send_all : can_send_pipe;
			pipe_list_it_t_next(&it, &receiver);
		}
	}
	// if there's no one to send to then in pub/sub we're good
	return can_send_all;
}

error_t obj_create_pipe(
		process_t * const process,
		object_number_t * objectno,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	object_pipe_t * no = NULL;
	error_t result = NO_ERROR;

#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Creating pipe named %s direction %d\n", name, direction);
#endif

	if (process && objectno)
	{
		*objectno = INVALID_OBJECT_ID;
		object_table_t * const table = process_get_object_table(process);
		if (table)
		{
			mem_pool_info_t * const pool = process_get_mem_pool(process);
			uint8_t * memory = NULL;
			pipe_list_t * tx_queue = NULL;
			pipe_list_t * rx_queue = NULL;
			switch (direction)
			{
			default:
			case PIPE_DIRECTION_UNKNOWN:
				result = PARAMETERS_INVALID;
				break;
			case PIPE_SEND_RECEIVE:
				tx_queue = pipe_list_t_create(pool);
				if (!tx_queue)
				{
					result = OUT_OF_MEMORY;
				}
				/* no break */
			case PIPE_RECEIVE:
			{
				if ((message_size % 4) != 0)
				{
					result = PIPE_MESSAGE_SIZE_NOT_ALIGNED;
				}
				uint32_t total_size;
				if (result == NO_ERROR)
				{
					total_size = (message_size * messages) + (messages * sizeof(uint32_t));
					while ((total_size % MMU_PAGE_SIZE) != 0)
					{
						total_size++;
					}
					memory = (uint8_t*)mem_alloc_aligned(
							pool, total_size, MMU_PAGE_SIZE);
					if (!memory)
					{
						result = OUT_OF_MEMORY;
					}
				}
#if defined(PIPE_DEBUGGING)
				debug_print("Pipe: Memory at %x\n", memory);
#endif
				if (result == NO_ERROR)
				{
					util_memset(memory, 0, total_size);
					rx_queue = pipe_list_t_create(pool);
					if (!rx_queue)
					{
						result = OUT_OF_MEMORY;
					}
				}
			}
				break;
			case PIPE_SEND:
				tx_queue = pipe_list_t_create(pool);
				if (!tx_queue)
				{
					result = OUT_OF_MEMORY;
				}
				break;
			}
			if (result == NO_ERROR)
			{
				no = (object_pipe_t*)mem_alloc(pool, sizeof(object_pipe_t));
#if defined(PIPE_DEBUGGING)
				debug_print("Creating pipe at %x\n", no);
#endif
				object_number_t objno;
				result = obj_add_object(table, (object_t*)no, &objno);
				if (result == NO_ERROR)
				{
					*objectno = objno;
					obj_initialise_object(&no->object, objno, PIPE_OBJ);
					no->direction = direction;
					no->pool = pool;
					no->memory = memory;
					no->tx_data.readers = rx_queue;
					no->rx_data.senders = tx_queue;
					util_memset(no->name, 0, MAX_SHARED_OBJECT_NAME_LENGTH);
					util_memcpy(no->name, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));
					const rx_data_t rx_data = {
							.senders = rx_queue,
							.free_messages = messages,
							.total_messages = messages,
							.message_size = message_size,
							.read_msg_position = 0,
							.write_msg_position = 0,
							.read_msg_ptr = no->memory,
							.write_msg_ptr = no->memory};
					const tx_data_t tx_data = {
							.readers = tx_queue,
							.sending_thread = NULL};
					no->rx_data = rx_data;
					no->tx_data = tx_data;
					// register it - create
					result = regsitery_add(name, process, no->object.object_number);
				}
				else
				{
					mem_free(pool, no);
				}
			}
			else
			{
				result = OUT_OF_MEMORY;
			}
		}
		else
		{
			result = PARAMETERS_OUT_OF_RANGE;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

error_t obj_open_pipe(
		process_t * const process,
		object_thread_t * const thread,
		object_number_t * objectno,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	object_pipe_t * no = NULL;
	error_t result;

#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Opening pipe %s direction %d\n", name, direction);
#endif

	process_t * other_proc = NULL;
	object_pipe_t * other_pipe = NULL;
	object_number_t other_pipe_no = INVALID_OBJECT_ID;
	result = registry_get(name, &other_proc, &other_pipe_no);
	// check that the pipe is in the registry
	if (result == NO_ERROR)
	{
		const object_table_t * const table = process_get_object_table(other_proc);
		object_t * const other_obj = obj_get_object(table, other_pipe_no);
		other_pipe = obj_cast_pipe(other_obj);

		if (other_obj)
		{
			// check that the named object actually is a pipe
			if (other_pipe)
			{
#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Opening pipe %s other pipe at %x direction is %d\n", name, other_pipe, other_pipe->direction);
#endif
				// we have a pipe! check that the direction is correct
				switch(direction)
				{
				default:
				case PIPE_DIRECTION_UNKNOWN:
					result = PARAMETERS_INVALID;
					break;
				case PIPE_SEND:
					if (!pipe_is_receiver(other_pipe))
					{
						result = PIPE_POLARITY_WRONG;
					}
					break;
				case PIPE_RECEIVE:
					if (!pipe_is_sender(other_pipe))
					{
						result = PIPE_POLARITY_WRONG;
					}
					break;
				case PIPE_SEND_RECEIVE:
					if (!(pipe_is_receiver(other_pipe) || pipe_is_sender(other_pipe)))
					{
						result = PIPE_POLARITY_WRONG;
					}
					break;
				}
			}
			else
			{
				result = WRONG_OBJ_TYPE;
			}
		}
		else
		{
			result = INVALID_OBJECT;
		}
	}
	else
	{
		registry_wait_for(thread, name);
		result = BLOCKED_RETRY;
	}
#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Open direction check result %d\n", result);
#endif
	if (result == NO_ERROR)
	{
		if (process && objectno)
		{
			object_table_t * const table = process_get_object_table(process);
			if (table)
			{
				mem_pool_info_t * const pool = process_get_mem_pool(process);
				uint8_t * memory = NULL;
				pipe_list_t * tx_queue = NULL;
				pipe_list_t * rx_queue = NULL;
				switch (direction)
				{
				default:
				case PIPE_DIRECTION_UNKNOWN:
					result = PARAMETERS_INVALID;
					break;
				case PIPE_SEND_RECEIVE:
					tx_queue = pipe_list_t_create(pool);
					if (!tx_queue)
					{
						result = OUT_OF_MEMORY;
					}
					/* no break */
				case PIPE_RECEIVE:
				{
					uint32_t total_size = (message_size * messages);
					while ((total_size % MMU_PAGE_SIZE) != 0)
					{
						total_size++;
					}
					memory = (uint8_t*)mem_alloc_aligned(
							pool, total_size, MMU_PAGE_SIZE);
					if (!memory)
					{
						result = OUT_OF_MEMORY;
					}
#if defined(PIPE_DEBUGGING)
					debug_print("Pipe: Memory at %x\n", memory);
#endif
					rx_queue = pipe_list_t_create(pool);
					if (!rx_queue)
					{
						result = OUT_OF_MEMORY;
					}
				}
					break;
				case PIPE_SEND:
					tx_queue = pipe_list_t_create(pool);
					if (!tx_queue)
					{
						result = OUT_OF_MEMORY;
					}
					break;
				}
#if defined(PIPE_DEBUGGING)
				debug_print("Pipe: Open setup result %d\n", result);
#endif
				if (result == NO_ERROR)
				{
					no = (object_pipe_t*)mem_alloc(pool, sizeof(object_pipe_t));
					object_number_t objno;
					result = obj_add_object(table, (object_t*)no, &objno);
					if (result == NO_ERROR)
					{
						*objectno = objno;
						obj_initialise_object(&no->object, objno, PIPE_OBJ);
						no->direction = direction;
						no->pool = pool;
						no->memory = memory;
						util_memset(no->name, 0, MAX_SHARED_OBJECT_NAME_LENGTH);
						util_memcpy(no->name, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));
						const rx_data_t rx_data = {
								.senders = rx_queue,
								.free_messages = messages,
								.total_messages = messages,
								.message_size = message_size,
								.read_msg_position = 0,
								.write_msg_position = 0,
								.read_msg_ptr = no->memory,
								.write_msg_ptr = no->memory};
						const tx_data_t tx_data = {
								.readers = tx_queue,
								.sending_thread = NULL};
						no->rx_data = rx_data;
						no->tx_data = tx_data;
						switch(direction)
						{
						case PIPE_DIRECTION_UNKNOWN:
							break;
						case PIPE_SEND:
							pipe_list_t_add(other_pipe->rx_data.senders, no);
							pipe_list_t_add(no->tx_data.readers, other_pipe);
							break;
						case PIPE_SEND_RECEIVE:
							pipe_list_t_add(other_pipe->tx_data.readers, no);
							pipe_list_t_add(other_pipe->rx_data.senders, no);
							pipe_list_t_add(no->tx_data.readers, other_pipe);
							pipe_list_t_add(no->rx_data.senders, other_pipe);
							break;
						case PIPE_RECEIVE:
							pipe_list_t_add(other_pipe->tx_data.readers, no);
							pipe_list_t_add(no->rx_data.senders, other_pipe);
							break;
						}
					}
					else
					{
#if defined(PIPE_DEBUGGING)
						debug_print("Pipe: Failed to add the pipe to the object table\n");
#endif
						mem_free(pool, no);
					}
				}
				else
				{
					result = OUT_OF_MEMORY;
				}
			}
			else
			{
				result = PARAMETERS_OUT_OF_RANGE;
			}
		}
		else
		{
			result = PARAMETERS_NULL;
		}
	}

	return result;
}

error_t obj_delete_pipe(object_pipe_t * const pipe)
{
	error_t result = NO_ERROR;

	if (pipe)
	{
		registry_remove(pipe->name);
		mem_free(pipe->pool, pipe->memory);
		mem_free(pipe->pool, pipe);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t obj_pipe_send_message(
		object_pipe_t * const pipe,
		object_thread_t * const thread,
		const tinker_pipe_send_kind_t send_kind,
		void * const message,
		const uint32_t message_size,
		const bool_t block)
{
	error_t result = NO_ERROR;

#if defined(PIPE_TRACING)
	debug_print("Pipe: Sending %d bytes\n", message_size);
#endif

	if (pipe)
	{
		if (pipe->direction == PIPE_SEND || pipe->direction == PIPE_SEND_RECEIVE)
		{
			if (send_kind == PIPE_TX_SEND_ALL && !pipe_can_send_to_all(pipe, block))
			{
#if defined(PIPE_DEBUGGING)
				debug_print("Pipe: Can't send to all receivers\n");
#endif
				// we can't send to everyone
				if (block)
				{
					pipe->tx_data.sending_thread = thread;
					obj_set_thread_waiting(thread, (object_t*)pipe);
					result = BLOCKED_RETRY;
				}
				else
				{
					result = PIPE_RECEIVERS_FULL;
				}
			}
		}
		else
		{
			result = PIPE_POLARITY_WRONG;
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	if (result == NO_ERROR)
	{
		pipe_list_it_t it;
		object_pipe_t * receiver = NULL;

		pipe_list_it_t_initialise(&it, pipe->tx_data.readers);
		const bool_t has_any_receivers = pipe_list_it_t_get(&it, &receiver);
		if (has_any_receivers)
		{
#if defined(PIPE_TRACING)
			debug_print("Pipe: There are receivers waiting\n");
#endif
			while(receiver)
			{
				if (pipe_can_receive(receiver))
				{
#if defined(PIPE_TRACING)
					debug_print("Pipe: Receiver is ready to receive the message\n");
#endif
					pipe_receive_message(receiver, message, message_size);
				}
				pipe_list_it_t_next(&it, &receiver);
			}
		}
	}

	return result;
}

error_t obj_pipe_receive_message(
		object_pipe_t * const pipe,
		object_thread_t * const thread,
		void ** const message,
		uint32_t ** const message_size,
		const bool_t block)
{
	error_t result = NO_ERROR;

	if (pipe)
	{
#if defined(PIPE_DEBUGGING)
		debug_print("Pipe: Recieving message, buffer at %x\n", pipe->memory);
#endif
		if (message && message_size)
		{
			if (pipe->direction == PIPE_RECEIVE || pipe->direction == PIPE_SEND_RECEIVE)
			{
				*message = NULL;
				*message_size = 0;

				const bool_t messages_in_buffer =
						pipe->rx_data.free_messages < pipe->rx_data.total_messages;
#if defined(PIPE_DEBUGGING)
				debug_print("Pipe: Receiver has free messages? %d. %d free, %d total\n",
						messages_in_buffer,
						pipe->rx_data.free_messages,
						pipe->rx_data.total_messages);
#endif
				if (!messages_in_buffer)
				{
					if (block)
					{
						*message = pipe->rx_data.read_msg_ptr + sizeof(uint32_t);
						*message_size = (uint32_t*)pipe->rx_data.read_msg_ptr;
						obj_set_thread_waiting(thread, (object_t*)pipe);
						pipe->rx_data.blocked_owner = thread;
						if (process_is_kernel(thread_get_parent(obj_get_thread(thread))))
						{
							tgt_wait_for_interrupt();
						}
					}
					else
					{
						result = PIPE_EMPTY;
					}
				}
				else
				{
#if defined(PIPE_DEBUGGING)
					debug_print("Pipe: Current buffer at at %x\n", pipe->rx_data.read_msg_ptr);
#endif
					*message = pipe->rx_data.read_msg_ptr + sizeof(uint32_t);
					*message_size = (uint32_t*)pipe->rx_data.read_msg_ptr;
				}
			}
			else
			{
				result = PIPE_POLARITY_WRONG;
			}
		}
		else
		{
			result = PARAMETERS_INVALID;
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t obj_pipe_received_message(object_pipe_t * const pipe)
{
	error_t result = NO_ERROR;

#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Acknowledging message received\n");
#endif

	if (pipe)
	{
		pipe->rx_data.free_messages++;

		if ((pipe->rx_data.read_msg_position + 1) == pipe->rx_data.total_messages)
		{
#if defined(PIPE_TRACING)
	debug_print("Pipe: Resettng to start\n");
#endif
			pipe->rx_data.read_msg_position = 0;
			pipe->rx_data.read_msg_ptr = pipe->memory;
		}
		else
		{
			pipe->rx_data.read_msg_position++;
			pipe->rx_data.read_msg_ptr += (pipe->rx_data.message_size+sizeof(uint32_t));
#if defined(PIPE_DEBUGGING)
	debug_print("Pipe: Pipe read position moved to %d\n", pipe->rx_data.read_msg_position);
#endif
		}

		// notify any senders
		pipe_list_it_t it;
		object_pipe_t * sender = NULL;
		pipe_list_it_t_initialise(&it, pipe->rx_data.senders);
		const bool_t has_any_senders = pipe_list_it_t_get(&it, &sender);
		if (has_any_senders)
		{
#if defined(PIPE_TRACING)
			debug_print("Pipe: Has senders to notify\n");
#endif
			while(sender)
			{
#if defined(PIPE_TRACING)
				debug_print("Pipe: Notifying sender\n");
#endif
				obj_set_thread_ready(sender->tx_data.sending_thread);
				pipe_list_it_t_next(&it, &sender);
			}
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}
#if defined(PIPE_TRACING)
	debug_print("Pipe: Results %d\n", result);
#endif
	return result;
}

const char * obj_pipe_get_name(const object_pipe_t * const pipe)
{
	const char * name = 0;
	if (pipe)
	{
		name = pipe->name;
	}
	return name;
}

tinker_pipe_direction_t obj_pipe_get_direction(const object_pipe_t * const pipe)
{
	tinker_pipe_direction_t dir = PIPE_DIRECTION_UNKNOWN;
	if (pipe)
	{
		dir = pipe->direction;
	}
	return dir;
}

uint32_t obj_pipe_get_total_messages(const object_pipe_t * const pipe)
{
	uint32_t v = 0;
	if (pipe)
	{
		v = pipe->rx_data.total_messages;
	}
	return v;
}

uint32_t obj_pipe_get_free_messages(const object_pipe_t * const pipe)
{
	uint32_t v = 0;
	if (pipe)
	{
		v = pipe->rx_data.free_messages;
	}
	return v;
}

uint32_t obj_pipe_get_msg_size(const object_pipe_t * const pipe)
{
	uint32_t v = 0;
	if (pipe)
	{
		v = pipe->rx_data.message_size;
	}
	return v;
}

uint32_t obj_pipe_get_read_msg_pos(const object_pipe_t * const pipe)
{
	uint32_t v = 0;
	if (pipe)
	{
		v = pipe->rx_data.read_msg_position;
	}
	return v;
}

uint32_t obj_pipe_get_write_msg_pos(const object_pipe_t * const pipe)
{
	uint32_t v = 0;
	if (pipe)
	{
		v = pipe->rx_data.write_msg_position;
	}
	return v;
}
