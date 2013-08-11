/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_pipe.h"

#include "arch/tgt_types.h"
#include "object_private.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/collections/unbounded_list.h"
#include "kernel/utils/collections/unbounded_list_iterator.h"

UNBOUNDED_LIST_TYPE(pipe_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_SPEC_ADD(static, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_BODY_CREATE(static, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_BODY_ADD(static, pipe_list_t, __object_pipe_t*)

UNBOUNDED_LIST_ITERATOR_TYPE(pipe_list_it_t)
UNBOUNDED_LIST_ITERATOR_INTERNAL_TYPE(pipe_list_it_t, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_ITERATOR_SPEC(static, pipe_list_it_t, pipe_list_t, __object_pipe_t*)
UNBOUNDED_LIST_ITERATOR_BODY(static, pipe_list_it_t, pipe_list_t, __object_pipe_t*)

typedef struct rx_data
{
	uint32_t free_messages;
	uint32_t total_messages;
	uint32_t current_message;
	uint32_t message_size;
	__object_thread_t * blocked_owner;
	uint8_t* current_message_ptr;
	pipe_list_t * senders;
} rx_data_t;

typedef struct tx_data
{
	__object_thread_t * sending_thread;
	pipe_list_t * listening_readers;
} tx_data_t;

typedef struct __object_pipe_t
{
	__object_internal_t object;
	registry_key_t name;
	sos_pipe_direction_t direction;
	rx_data_t rx_data;
	tx_data_t tx_data;
	__mem_pool_info_t * pool;
	uint8_t * memory;
} __object_pipe_internal_t;

__object_pipe_t * __obj_cast_pipe(const __object_t * const o)
{
	__object_pipe_t * result = NULL;
	if(o)
	{
		const __object_pipe_t * const tmp = (const __object_pipe_t*)o;
		if (tmp->object.initialised == OBJECT_INITIALISED
			&& tmp->object.type == PIPE_OBJ)
		{
			result = (__object_pipe_t*)tmp;
		}
	}
	return result;
}

object_number_t __obj_pipe_get_oid
	(const __object_pipe_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		if (o->object.initialised == OBJECT_INITIALISED)
		{
			oid = o->object.object_number;
		}
	}
	return oid;
}

static bool_t __pipe_can_receive(const __object_pipe_t * const pipe)
{
	return pipe
			&& ((pipe->direction == PIPE_RECEIVE || pipe->direction == PIPE_SEND_RECEIVE)
			&& (pipe->rx_data.free_messages > 0));
}

static void __pipe_receive_message(
		const __object_pipe_t * const sender,
		__object_pipe_t * const receiver,
		const void * message,
		const uint32_t message_size)
{
	uint32_t * const msg_size = (uint32_t*)receiver->rx_data.current_message_ptr;
	*msg_size = message_size;
	__util_memcpy(receiver->rx_data.current_message_ptr+4, message, message_size);
	if ((receiver->rx_data.current_message + 1) == receiver->rx_data.total_messages)
	{
		receiver->rx_data.current_message = 0;
		receiver->rx_data.current_message_ptr = receiver->memory;
	}
	else
	{
		receiver->rx_data.current_message++;
		receiver->rx_data.current_message_ptr += (receiver->rx_data.message_size+4);
	}
	receiver->rx_data.free_messages--;
	if (__obj_thread_is_waiting_on(receiver->rx_data.blocked_owner, (__object_t*)sender))
	{

		__obj_set_thread_ready(receiver->rx_data.blocked_owner);
		receiver->rx_data.blocked_owner = NULL;
	}
}

static bool_t __pipe_is_receiver(const __object_pipe_t * const pipe)
{
	return ((pipe->direction == PIPE_RECEIVE) || (pipe->direction == PIPE_SEND_RECEIVE));
}

static bool_t __pipe_is_sender(const __object_pipe_t * const pipe)
{
	return ((pipe->direction == PIPE_SEND) || (pipe->direction == PIPE_SEND_RECEIVE));
}

static bool_t __pipe_is_receiver_sender(const __object_pipe_t * const pipe)
{
	return (pipe->direction == PIPE_SEND_RECEIVE);
}

static bool_t __pipe_can_send_to_all(
		const __object_pipe_t * const pipe,
		const bool_t blocking)
{
	pipe_list_it_t it;
	__object_pipe_t * receiver = NULL;
	bool_t can_send_all = true;

	pipe_list_it_t_initialise(&it, pipe->tx_data.listening_readers);
	const bool_t has_any_receivers = pipe_list_it_t_get(&it, &receiver);
	if (has_any_receivers)
	{
		while(receiver)
		{
			can_send_all = __pipe_can_receive(receiver);
			if (!can_send_all && blocking)
			{
				pipe_list_t_add(receiver->rx_data.senders, (__object_pipe_t*)pipe);
			}
			pipe_list_it_t_next(&it, &receiver);
		}
	}
	// if there's no one to send to then in pub/sub we're good
	return can_send_all;
}

error_t __obj_create_pipe(
		__process_t * const process,
		object_number_t * objectno,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	__object_pipe_t * no = NULL;
	error_t result = NO_ERROR;

	if (process && objectno)
	{
		*objectno = INVALID_OBJECT_ID;
		__object_table_t * const table = __process_get_object_table(process);
		if (table)
		{
			__mem_pool_info_t * const pool = __process_get_mem_pool(process);
			uint8_t * memory = NULL;
			pipe_list_t * tx_queue = NULL;
			pipe_list_t * rx_queue = NULL;
			switch (direction)
			{
			case PIPE_DIRECTION_UNKNOWN:
				result = PARAMETERS_INVALID;
				break;
			case PIPE_SEND_RECEIVE:
			case PIPE_RECEIVE:
			{
				uint32_t total_size = (message_size * messages) + (messages * 4);
				while ((total_size % MMU_PAGE_SIZE) != 0)
				{
					total_size++;
				}
				memory = (uint8_t*)__mem_alloc_aligned(
						pool, total_size, MMU_PAGE_SIZE);
				if (!memory)
				{
					result = OUT_OF_MEMORY;
				}
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
			if (result == NO_ERROR)
			{
				no = (__object_pipe_t*)__mem_alloc(pool, sizeof(__object_pipe_t));
				object_number_t objno;
				result = __obj_add_object(table, (__object_t*)no, &objno);
				if ( result == NO_ERROR )
				{
					*objectno = objno;
					__obj_initialise_object(&no->object, objno, PIPE_OBJ);
					no->direction = direction;
					no->pool = pool;
					no->memory = memory;
					no->tx_data.listening_readers = rx_queue;
					no->rx_data.senders = tx_queue;
					memset(no->name, 0, sizeof(registry_key_t));
					__util_memcpy(no->name, name, __util_strlen(name, sizeof(registry_key_t)));
					const rx_data_t counter = {
							.free_messages = messages,
							.total_messages = messages,
							.message_size = message_size,
							.current_message = 0,
							.current_message_ptr = no->memory};
					no->rx_data = counter;
					// register it - create
					result = __regsitery_add(name, process, no->object.object_number);
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

error_t __object_open_pipe(
		__process_t * const process,
		object_number_t * objectno,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	__object_pipe_t * no = NULL;
	error_t result = NO_ERROR;

	__process_t * other_proc = NULL;
	__object_pipe_t * other_pipe = NULL;
	object_number_t other_pipe_no = INVALID_OBJECT_ID;
	result = __registry_get(name, &other_proc, &other_pipe_no);
	// check that the pipe is in the registry
	if (result == NO_ERROR)
	{
		const __object_table_t * const table = __process_get_object_table(other_proc);
		const __object_t * const other_obj = __obj_get_object(table, other_pipe_no);
		other_pipe = __obj_cast_pipe(other_obj);

		// check that the named object actually is a pipe
		if (other_pipe)
		{
			// we have a pipe! check that the direction is correct
			switch(direction)
			{
			case PIPE_DIRECTION_UNKNOWN:
				result = PARAMETERS_INVALID;
				break;
			case PIPE_SEND:
				if (!__pipe_is_receiver(other_pipe))
				{
					result = PIPE_POLARITY_WRONG;
				}
				break;
			case PIPE_RECEIVE:
				if (!__pipe_is_sender(other_pipe))
				{
					result = PIPE_POLARITY_WRONG;
				}
				break;
			case PIPE_SEND_RECEIVE:
				if (!__pipe_is_receiver_sender(other_pipe))
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
		result = UNKNOWN_PIPE;
	}

	if (result == NO_ERROR)
	{
		if (process && objectno)
		{
			__object_table_t * table = __process_get_object_table(process);
			if (table)
			{
				__mem_pool_info_t * const pool = __process_get_mem_pool(process);
				uint8_t * memory = NULL;
				pipe_list_t * queue = NULL;
				switch (direction)
				{
				case PIPE_DIRECTION_UNKNOWN:
					result = PARAMETERS_INVALID;
					break;
				case PIPE_SEND_RECEIVE:
					queue = pipe_list_t_create(pool);
					if (!queue)
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
					memory = (uint8_t*)__mem_alloc_aligned(
							pool, total_size, MMU_PAGE_SIZE);
					if (!memory)
					{
						result = OUT_OF_MEMORY;
					}
				}
					break;
				case PIPE_SEND:
					queue = pipe_list_t_create(pool);
					if (!queue)
					{
						result = OUT_OF_MEMORY;
					}
					break;
				}
				if (result == NO_ERROR)
				{
					no = (__object_pipe_t*)__mem_alloc(pool, sizeof(__object_pipe_t));
					object_number_t objno;
					result = __obj_add_object(table, (__object_t*)no, &objno);
					if ( result == NO_ERROR )
					{
						*objectno = objno;
						__obj_initialise_object(&no->object, objno, PIPE_OBJ);
						no->direction = direction;
						no->pool = pool;
						no->memory = memory;
						no->tx_data.listening_readers = queue;
						memset(no->name, 0, sizeof(registry_key_t));
						__util_memcpy(no->name, name, __util_strlen(name, sizeof(registry_key_t)));
						const rx_data_t counter = {
								.free_messages = messages,
								.total_messages = messages,
								.message_size = message_size,
								.current_message = 0,
								.current_message_ptr = no->memory};
						no->rx_data = counter;
						switch(direction)
						{
						case PIPE_DIRECTION_UNKNOWN:
							break;
						case PIPE_SEND:
							break;
						case PIPE_SEND_RECEIVE:
						case PIPE_RECEIVE:
							pipe_list_t_add(other_pipe->tx_data.listening_readers, no);
							break;
						}
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

error_t __obj_delete_pipe(__object_pipe_t * const pipe)
{
	error_t result = NO_ERROR;

	if (pipe)
	{
		__registry_remove(pipe->name);
		__mem_free(pipe->pool, pipe->memory);
		__mem_free(pipe->pool, pipe);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_pipe_send_message(
		__object_pipe_t * const pipe,
		__object_thread_t * const thread,
		const sos_pipe_send_kind_t send_kind,
		void * const message,
		const uint32_t message_size,
		const bool_t block)
{
	error_t result = NO_ERROR;

	if (pipe)
	{
		if (send_kind == PIPE_TX_SEND_ALL && !__pipe_can_send_to_all(pipe, block))
		{
			// we can't send to everyone
			if (block)
			{
				pipe->tx_data.sending_thread = thread;
				__obj_set_thread_waiting(thread, (__object_t*)pipe);
				result = PIPE_SEND_BLOCKED;
			}
			else
			{
				result = PIPE_RECEIVERS_FULL;
			}
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	if (result == NO_ERROR)
	{
		pipe_list_it_t it;
		__object_pipe_t * receiver = NULL;

		pipe_list_it_t_initialise(&it, pipe->tx_data.listening_readers);
		const bool_t has_any_receivers = pipe_list_it_t_get(&it, &receiver);
		if (has_any_receivers)
		{
			while(receiver)
			{
				if (__pipe_can_receive(receiver))
				{
					__pipe_receive_message(pipe, receiver, message, message_size);
				}
				pipe_list_it_t_next(&it, &receiver);
			}
		}
	}

	return result;
}

error_t __obj_pipe_receive_message(
		__object_pipe_t * const pipe,
		__object_thread_t * const thread,
		void ** const message,
		uint32_t * const message_size,
		const bool_t block)
{
	error_t result = NO_ERROR;
	if (pipe)
	{
		if (message && message_size)
		{
			*message = NULL;
			*message_size = 0;

			const bool_t messages_in_buffer =
					pipe->rx_data.free_messages < pipe->rx_data.total_messages;
			if (!messages_in_buffer)
			{
				if (block)
				{
					*message = pipe->rx_data.current_message_ptr + 4;
					*message_size = *(uint32_t*)pipe->rx_data.current_message_ptr;
					__obj_set_thread_waiting(thread, (__object_t*)pipe);
				}
				else
				{
					result = PIPE_EMPTY;
				}
			}
			else
			{
				*message = pipe->rx_data.current_message_ptr + 4;
				*message_size = *(uint32_t*)pipe->rx_data.current_message_ptr;
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

error_t __obj_pipe_received_message(__object_pipe_t * const pipe)
{
	error_t result = NO_ERROR;

	if (pipe)
	{
		pipe->rx_data.free_messages++;

		// notify any senders
		pipe_list_it_t it;
		__object_pipe_t * sender = NULL;
		pipe_list_it_t_initialise(&it, pipe->rx_data.senders);
		const bool_t has_any_senders = pipe_list_it_t_get(&it, &sender);
		if (has_any_senders)
		{
			while(sender)
			{
				__obj_set_thread_ready(sender->tx_data.sending_thread);
				pipe_list_it_t_next(&it, &sender);
			}
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}
