/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/obj_pipe.h"

#include "tgt_types.h"
#include "tgt.h"
#include "objects/object_private.h"
#include "utils/util_strlen.h"
#include "utils/collections/unbounded_list.h"
#include "utils/collections/unbounded_list_iterator.h"

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

typedef struct rx_blocked
{
	object_thread_t * blocked_owner;
	void * message;
	uint32_t * message_size;
} rx_blocked_t;

typedef struct rx_data
{
	uint32_t free_messages;
	uint32_t total_messages;
	uint32_t read_msg_position;
	uint32_t write_msg_position;
	uint32_t message_size;
	rx_blocked_t blocked_info;
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
	registry_t * reg;
	char name[MAX_SHARED_OBJECT_NAME_LENGTH];
} object_pipe_internal_t;

static return_t obj_pipe_received_message(object_pipe_t * const pipe);

object_pipe_t * obj_cast_pipe(object_t * const o)
{
	object_pipe_t * result = NULL;
	if (o)
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

static uint32_t align(uint32_t a, uint32_t b)
{
	uint32_t aligned_size;
	if (a % b == 0)
	{
		aligned_size = a;
	}
	else
	{
		aligned_size = a + (b - (a % b));
	}
	return aligned_size;
}

static void pipe_receive_message(
		object_pipe_t * const receiver,
		const void * message,
		const uint32_t message_size)
{
	if (is_debug_enabled(PIPE))
	{
		debug_print(PIPE, "PipeW: Writing size (%d) to %x and data to %x\n",
				message_size,
				receiver->rx_data.write_msg_ptr,
				receiver->rx_data.write_msg_ptr+sizeof(uint32_t));
	}
	// copy the message size over
	uint32_t * const msg_size = (uint32_t*)receiver->rx_data.write_msg_ptr;
	*msg_size = message_size;
	// copy the the payload over (bytes, not 32)
	util_memcpy(receiver->rx_data.write_msg_ptr+sizeof(uint32_t), message, message_size);
	// ring buffer; if at the end, return to home
	if ((receiver->rx_data.write_msg_position + 1) == receiver->rx_data.total_messages)
	{
		receiver->rx_data.write_msg_position = 0;
		receiver->rx_data.write_msg_ptr = receiver->memory;
	}
	else
	{
		// write the buffer size
		receiver->rx_data.write_msg_position++;
		// FIXME this must be word aligned for the MMU - message_size maybe un-aligned
		uint32_t aligned_size = align(receiver->rx_data.message_size, sizeof(uint32_t));
		debug_print(PIPE, "Increasing by 4 + aligned size of %d\n", aligned_size);
		receiver->rx_data.write_msg_ptr += (aligned_size + sizeof(uint32_t));
	}
#if defined (KERNEL_DEBUGGING)
	const uint32_t old_free = receiver->rx_data.free_messages;
#endif
	receiver->rx_data.free_messages--;
	if (is_debug_enabled(PIPE_TRACE))
	{
		debug_print(PIPE_TRACE, "PipeW: Receiver has %d -> %d free messages\n", old_free, receiver->rx_data.free_messages);
		debug_print(PIPE_TRACE, "PipeW: Receiver has thread %x blocking on %x\n",
			(receiver->rx_data.blocked_info.blocked_owner),
			receiver);
	}
	if (obj_thread_is_waiting_on(receiver->rx_data.blocked_info.blocked_owner, (object_t*)receiver))
	{
		debug_prints(PIPE_TRACE, "PipeW: Blocked owner - resuming them\n");
		obj_set_thread_ready(receiver->rx_data.blocked_info.blocked_owner);
		receiver->rx_data.blocked_info.blocked_owner = NULL;
		util_memcpy(receiver->rx_data.blocked_info.message_size, receiver->rx_data.read_msg_ptr, sizeof(uint32_t));
		util_memcpy(receiver->rx_data.blocked_info.message, receiver->rx_data.read_msg_ptr + sizeof(uint32_t), *receiver->rx_data.blocked_info.message_size);
		obj_pipe_received_message(receiver);
		receiver->rx_data.blocked_info.message = NULL;
		receiver->rx_data.blocked_info.message_size = NULL;
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
		while (receiver)
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

return_t obj_create_pipe(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	object_pipe_t * no = NULL;
	return_t result = NO_ERROR;

	debug_print(PIPE_TRACE, "PipeC: Creating pipe named %s direction %d\n", name, direction);

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
				// fall through
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
							process_get_user_mem_pool(process), total_size, MMU_PAGE_SIZE);
					if (!memory)
					{
						result = OUT_OF_MEMORY;
					}
				}
				debug_print(PIPE_TRACE, "PipeC: Memory at %x\n", memory);
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
				debug_print(PIPE_TRACE, "PipeC: Creating pipe at %x\n", no);
				object_number_t objno;
				result = obj_add_object(table, (object_t*)no, &objno);
				if (result == NO_ERROR)
				{
					*objectno = objno;
					obj_initialise_object(&no->object, objno, PIPE_OBJ);
					no->direction = direction;
					no->pool = pool;
					no->memory = memory;
					no->reg = reg;
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
					result = regsitery_add(reg, name, process, no->object.object_number);
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

return_t obj_open_pipe(
		registry_t * const reg,
		process_t * const process,
		object_thread_t * const thread,
		object_number_t * objectno,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	object_pipe_t * no = NULL;
	return_t result;

	debug_print(PIPE_TRACE, "PipeO: Opening pipe %s direction %d\n", name, direction);

	process_t * other_proc = NULL;
	object_pipe_t * other_pipe = NULL;
	object_number_t other_pipe_no = INVALID_OBJECT_ID;
	result = registry_get(reg, name, &other_proc, &other_pipe_no);
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
				debug_print(PIPE_TRACE, "PipeO: Opening pipe %s other pipe at %x direction is %d\n", name, other_pipe, other_pipe->direction);
				// we have a pipe! check that the direction is correct
				switch (direction)
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
		registry_wait_for(reg, thread, name);
		result = BLOCKED_RETRY;
	}
	debug_print(PIPE_TRACE, "PipeO: Open direction check result %d\n", result);
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
					// fall through
				case PIPE_RECEIVE:
				{
					uint32_t total_size = (message_size * messages);
					while ((total_size % MMU_PAGE_SIZE) != 0)
					{
						total_size++;
					}
					memory = (uint8_t*)mem_alloc_aligned(
							process_get_user_mem_pool(process), total_size, MMU_PAGE_SIZE);
					if (memory)
					{
						util_memset(memory, 0, total_size);
					}
					else
					{
						result = OUT_OF_MEMORY;
					}
					debug_print(PIPE_TRACE, "PipeO: Memory at %x\n", memory);
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
				debug_print(PIPE_TRACE, "PipeO: Open setup result %d\n", result);
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
						no->reg = reg;
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
						debug_prints(PIPE_TRACE, "PipeO: Failed to add the pipe to the object table\n");
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

return_t obj_delete_pipe(object_pipe_t * const pipe)
{
	return_t result = NO_ERROR;

	if (pipe)
	{
		registry_remove(pipe->reg, pipe->name);
		mem_free(pipe->pool, pipe->memory);
		mem_free(pipe->pool, pipe);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

return_t obj_pipe_send_message(
		object_pipe_t * const pipe,
		object_thread_t * const thread,
		const tinker_pipe_send_kind_t send_kind,
		void * const message,
		const uint32_t message_size,
		const bool_t block)
{
	return_t result = NO_ERROR;

	debug_print(PIPE_TRACE, "PipeW: Sending %d bytes to pipe %x\n", message_size, pipe);
	if (pipe)
	{
		debug_print(PIPE_TRACE, "PipeW: Direction %d\n", pipe->direction);
		if (pipe->direction == PIPE_SEND || pipe->direction == PIPE_SEND_RECEIVE)
		{
			if (send_kind == PIPE_TX_SEND_ALL && !pipe_can_send_to_all(pipe, block))
			{
				debug_prints(PIPE_TRACE, "PipeW: Can't send to all receivers\n");
				// we can't send to everyone
				if (block)
				{
					pipe->tx_data.sending_thread = thread;
					debug_print(PIPE_TRACE, "PipeW: Thread %x blocking on pipe %x\n", thread, pipe);
					obj_set_thread_waiting(thread, (object_t*)pipe);
					result = BLOCKED_RETRY;
					// TODO need to block in kernel mode
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
			debug_prints(PIPE_TRACE, "PipeW: There are receivers waiting\n");
			while (receiver)
			{
				if (pipe_can_receive(receiver))
				{
					debug_print(PIPE_TRACE, "PipeW: Receiver %x is ready to receive the message\n", receiver);
					pipe_receive_message(receiver, message, message_size);
				}
				pipe_list_it_t_next(&it, &receiver);
			}
		}
	}

	return result;
}

return_t obj_pipe_receive_message(
		object_pipe_t * const pipe,
		object_thread_t * const thread,
		void * const message,
		uint32_t * const message_size,
		uint32_t max_size,
		const bool_t block)
{
	return_t result = NO_ERROR;

	if (pipe)
	{
		debug_print(PIPE, "PipeR: Recieving message, buffer at %x\n", pipe->memory);
		if (message && message_size)
		{
			if (pipe->direction == PIPE_RECEIVE || pipe->direction == PIPE_SEND_RECEIVE)
			{
				const bool_t messages_in_buffer = pipe->rx_data.free_messages < pipe->rx_data.total_messages;
				debug_print(PIPE_TRACE,
						"PipeR: Receiver has free messages? %d. %d free, %d total\n",
						messages_in_buffer,
						pipe->rx_data.free_messages,
						pipe->rx_data.total_messages);
				if (messages_in_buffer)
				{
					debug_print(PIPE, "PipeR: Current buffer at at %x\n", pipe->rx_data.read_msg_ptr);
					util_memcpy(message_size, pipe->rx_data.read_msg_ptr, sizeof(uint32_t));
					if (*message_size > max_size)
					{
						result = PARAMETERS_OUT_OF_RANGE;
					}
					else
					{
						util_memcpy(message, pipe->rx_data.read_msg_ptr + sizeof(uint32_t), *message_size);
						obj_pipe_received_message(pipe);
					}
				}
				else
				{
					if (block)
					{
						obj_set_thread_waiting(thread, (object_t*)pipe);
						debug_print(PIPE_TRACE, "PipeR: Thread %x blocking on pipe %x\n", thread, pipe);
						pipe->rx_data.blocked_info.blocked_owner = thread;
						pipe->rx_data.blocked_info.message = message;
						pipe->rx_data.blocked_info.message_size = message_size;
					}
					else
					{
						result = PIPE_EMPTY;
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
			result = PARAMETERS_INVALID;
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}
	return result;
}

static return_t obj_pipe_received_message(object_pipe_t * const pipe)
{
	return_t result = NO_ERROR;

	debug_prints(PIPE, "PipeA: Acknowledging message received\n");

	if (pipe)
	{
		pipe->rx_data.free_messages++;

		if ((pipe->rx_data.read_msg_position + 1) == pipe->rx_data.total_messages)
		{
			debug_prints(PIPE_TRACE, "PipeA: Resettng to start\n");
			pipe->rx_data.read_msg_position = 0;
			pipe->rx_data.read_msg_ptr = pipe->memory;
		}
		else
		{
			pipe->rx_data.read_msg_position++;
			pipe->rx_data.read_msg_ptr += (align(pipe->rx_data.message_size, sizeof(uint32_t)) + sizeof(uint32_t));
			debug_print(PIPE, "PipeA: Pipe read position moved to %d\n", pipe->rx_data.read_msg_position);
		}

		// notify any senders
		pipe_list_it_t it;
		object_pipe_t * sender = NULL;
		pipe_list_it_t_initialise(&it, pipe->rx_data.senders);
		const bool_t has_any_senders = pipe_list_it_t_get(&it, &sender);
		if (has_any_senders)
		{
			debug_prints(PIPE_TRACE, "PipeA: Has senders to notify\n");
			while (sender)
			{
				debug_prints(PIPE_TRACE, "PipeA: Notifying sender\n");
				obj_set_thread_ready(sender->tx_data.sending_thread);
				pipe_list_it_t_next(&it, &sender);
			}
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}
	debug_print(PIPE_TRACE, "PipeA: Results %d\n", result);
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
