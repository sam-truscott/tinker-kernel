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

typedef struct __object_pipe_t
{
	__object_internal_t object;
	__process_t * process; //TODO : process OBJECT not TYPE
	__object_pipe_direction_t direction;
	__mem_pool_info_t * pool;
	void * memory;
} __object_pipe_internal_t;

__object_pipe_t * __obj_cast_pipe(__object_t * o)
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

error_t __obj_create_pipe(
		__process_t * const process,
		__object_table_t * const table,
		object_number_t * objectno,
		const char * const name,
		const __object_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	__object_pipe_t * no = NULL;
	error_t result = NO_ERROR;

	if ( objectno )
	{
		if ( table )
		{
			__mem_pool_info_t * const pool = __process_get_mem_pool(process);
			no = (__object_pipe_t*)__mem_alloc(pool, sizeof(__object_pipe_t));
			object_number_t objno;
			result = __obj_add_object(table, (__object_t*)no, &objno);
			if ( result == NO_ERROR )
			{
				__obj_initialise_object(&no->object, objno, SEMAPHORE_OBJ);
				no->direction = direction;
				no->pool = pool;
				no->process = process;
				uint32_t total_size = (message_size * messages);
				while ((total_size % MMU_PAGE_SIZE) != 0)
				{
					total_size++;
				}
				no->memory = __mem_alloc_aligned(pool, total_size, MMU_PAGE_SIZE);
				result = __regsitery_add(name, no->process, no->object.object_number);
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

error_t __obj_delete_pipe(__object_pipe_t * const pipe)
{
	if (pipe){}
	return NO_ERROR;
}

error_t __obj_pipe_send_message(
		__object_pipe_t * const pipe,
		void * const message,
		const uint32_t message_size,
		const bool_t block)
{
	if (pipe && message && message_size && block) {}
	return NO_ERROR;
}

error_t __obj_pipe_receive_message(
		__object_pipe_t * const pipe,
		void * const message,
		uint32_t * const message_size,
		const uint32_t max_message_size,
		const bool_t block)
{
	if (pipe && message && message_size && max_message_size && block) {}
	return NO_ERROR;
}
