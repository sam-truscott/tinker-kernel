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
#include "kernel/kernel_assert.h"
#include "kernel/console/print_out.h"
#include "kernel/process/thread.h"
#include "kernel/process/process_manager.h"
#include "kernel/memory/memory_manager.h"

typedef struct __object_process_t
{
	__object_internal_t object;
	uint32_t pid;
	__mem_pool_info_t * pool;
	__process_t * process;
} __object_process_internal_t;

__object_process_t * __obj_cast_process(__object_t * o)
{
	__object_process_t * result = NULL;
	if(o)
	{
		const __object_process_t * const tmp = (const __object_process_t*)o;
		if (tmp->object.type == PROCESS_OBJ)
		{
			result = (__object_process_t*)tmp;
		}
	}
	return result;
}

error_t __obj_create_process(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		const uint32_t process_id,
		__process_t * const process,
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
				no->pid = process_id;
				no->pool = pool;
				no->process = process;
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

error_t __obj_process_thread_exit(
		__object_process_t * const o,
		__object_thread_t * const thread)
{
	error_t ret;
	__kernel_assert("__obj_process_thread - check process object exists\n", o != NULL);
	__kernel_assert("__obj_process_thread - check thread object exists\n", thread != NULL);

#if defined(__PROCESS_DEBUGGING)
		__debug_print("proc %d (%s) is exiting\n", o->pid, __process_get_image(o->process));
#endif

	__process_thread_exit(o->process, __obj_get_thread(thread));
	__obj_remove_object(
			__process_get_object_table(o->process),
			__obj_thread_get_oid(thread));

	ret = __obj_exit_thread(thread);

	const uint32_t thread_count = __process_get_thread_count(o->process);
	if (!thread_count)
	{
		__obj_process_exit(o);
	}

	return ret;
}

void __obj_process_exit(__object_process_t * const o)
{
	__kernel_assert("__obj_process_exit - check process object exists\n", o != NULL);
	__obj_remove_object(
			__process_get_object_table(o->process),
			o->object.object_number);
#if defined(__PROCESS_DEBUGGING)
	__debug_print("process %d exit\n", o->pid);
#endif
	__process_t * const proc = o->process;
	__obj_delete_process(o);
	__proc_delete_proc(proc);
	__process_exit(proc);
}

void __obj_delete_process(__object_process_t * const o)
{
	__mem_free(o->pool, o);
}

object_number_t __obj_process_get_oid
	(const __object_process_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

uint32_t __obj_process_pid(const __object_process_t * const o)
{
	return o->pid;
}
