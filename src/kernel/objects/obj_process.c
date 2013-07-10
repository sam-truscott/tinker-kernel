/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_process.h"
#include "object.h"
#include "object_private.h"
#include "object_table.h"
#include "kernel/memory/memory_manager.h"

typedef struct __object_process_t
{
	__object_internal_t object;
	uint32_t pid;
} __object_process_internal_t;

error_t __obj_create_process(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		const uint32_t process_id,
		__object_t ** object)
{
	__object_process_t * no = NULL;
	error_t result = NO_ERROR;

	if ( object )
	{
		if ( table)
		{
			no = (__object_process_t*)__mem_alloc(pool, sizeof(__object_process_t));
			object_number_t objno;
			result = __obj_add_object(table, (__object_t*)no, &objno);
			if ( result == NO_ERROR )
			{
				__obj_initialise_object(&no->object, objno, PROCESS_OBJ);
				__obj_lock(&no->object);
				no->pid = process_id;
				__obj_release(&no->object);
				*object = (__object_t*)no;
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
