/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "process_manager.h"

#include "config.h"
#include "arch/tgt.h"
#include "kernel/kernel_initialise.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_malloc.h"
#include "kernel/utils/util_free.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_process.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/debug/debug_print.h"

/**
 * A linked list of processes
 */
UNBOUNDED_LIST_TYPE(process_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(process_list_t, __process_t*)
UNBOUNDED_LIST_SPEC(static, process_list_t, __process_t*)
UNBOUNDED_LIST_BODY(static, process_list_t, __process_t*)

/**
 * The static list of processes in the system
 */
static process_list_t * __process_list = NULL;

void __proc_initialise(void)
{
	__process_list = process_list_t_create(__mem_get_default_pool());
}

error_t __proc_create_process(
		const char * image,
		const char * initial_task_name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t heap,
		const uint32_t flags,
		__process_t ** process)
{
	error_t ret = NO_ERROR;
	__process_t * proc = NULL;

	if ( process )
	{
		* process = 0;
	}

	/* get the new process id - SLOW! - TODO need to speed up */
	__process_t * tmp = NULL;
	uint32_t proc_id = 0;
	for ( uint32_t i = 0 ; i < __MAX_PROCESSES ; i++ )
	{
		if ( !process_list_t_get(__process_list, i, &tmp) )
		{
			proc_id = i;
			break;
		}
	}

	/*
	 * If the current thread is null we're in initialisation and
	 * it means we're creating the kernels process so we can
	 * use the default pool.
	 *
	 * Otherwise we need to use the current processes pool.
	 */
	const __thread_t * const curr_thread = __sch_get_current_thread();
	__mem_pool_info_t * parent_pool;
	if (curr_thread == NULL )
	{
#if defined (__PROCESS_DEBUGGING)
		__debug_print("proc: create process from default pool\n");
#endif
		parent_pool = __mem_get_default_pool();
	} else {
#if defined (__PROCESS_DEBUGGING)
		__debug_print("proc: create process for %s from parent\n",
				image);
#endif
		parent_pool = __process_get_mem_pool(__thread_get_parent(curr_thread));
	}

	__mem_pool_info_t * new_mem_pool = NULL;
	const bool pool_allocated = __mem_init_process_memory(
			parent_pool,
			&new_mem_pool,
			heap + stack);

	if (!pool_allocated)
	{
		ret = OUT_OF_MEMORY;
	}
	else
	{
		ret = __process_create(
				parent_pool,
				proc_id,
				image,
				(curr_thread == NULL),
				new_mem_pool,
				&proc);

		if (ret == NO_ERROR)
		{
			__object_t * process_obj = NULL;
			__object_t * thread_obj = NULL;
			ret = __proc_create_thread(
									proc,
									initial_task_name,
									entry_point,
									priority,
									stack,
									flags,
									&thread_obj,
									NULL);

			if ( ret == NO_ERROR )
			{
				ret = __obj_create_process(
						__process_get_mem_pool(proc),
						__process_get_object_table(proc),
						__process_get_pid(proc),
						proc,
						&process_obj);
				//FIXME: pass oid in as param
				__process_set_oid(proc, __obj_get_number(process_obj));

				if ( ret == NO_ERROR )
				{
					if ( process_list_t_add(__process_list, proc) == false )
					{
						__util_free(proc);
						ret = OUT_OF_MEMORY;
					}
					else
					{
						if ( process )
						{
							* process = proc;
						}
					}
				}
				else
				{
					ret = OBJECT_ADD_FAILED;
				}
			}
		}
	}

	return ret;
}

error_t __proc_create_thread(
		__process_t * process,
		const char * const name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t flags,
		__object_t ** thread_object,
		__thread_t ** new_thread)
{
	error_t ret = NO_ERROR;
	__thread_t * thread = NULL;

	/* allocate memory for thread from processes pool */
	thread = __thread_create(
			__process_get_mem_pool(process),
			(const __fwd_process_t*)process,
			priority,
			entry_point,
			flags,
			stack,
			name);

	/* initialise the thread */
	if ( thread != NULL )
	{
		/* add the thread to the process list */
		object_number_t objno = INVALID_OBJECT_ID;
		if (__process_add_thread(process, thread, &objno))
		{
			if (__thread_get_state(thread) != thread_ready)
			{
				ret = OUT_OF_MEMORY;
			}
		}
		else
		{
			ret = OUT_OF_MEMORY;
		}

		if ( ret == NO_ERROR )
		{
			if ( new_thread )
			{
				*new_thread = thread;
			}

			if ( thread_object )
			{
				*thread_object = __obj_get_object(
						__process_get_object_table(process),
						objno);
			}
		}
	}

	return ret;
}
