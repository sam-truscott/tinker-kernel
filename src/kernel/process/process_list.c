/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "process_list.h"

#include "config.h"
#include "arch/tgt.h"
#include "kernel/kernel_initialise.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_process.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/console/print_out.h"

/**
 * A linked list of processes
 */
UNBOUNDED_LIST_INTERNAL_TYPE(process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_GET(static, process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_ADD(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_CREATE(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_GET(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_ADD(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_REMOVE(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static, process_list_t, process_t*)

UNBOUNDED_LIST_ITERATOR_INTERNAL_TYPE(process_list_it_t, process_list_t, process_t*)
UNBOUNDED_LIST_ITERATOR_BODY(extern, process_list_it_t, process_list_t, process_t*)

typedef struct proc_list_t
{
	scheduler_t * scheduler;
	process_list_t * process_list;
	alarm_manager_t * alarm_manager;
} proc_list_t;

proc_list_t * proc_create(
		mem_pool_info_t * const pool,
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager)
{
	proc_list_t * const list = mem_alloc(pool, sizeof(process_list_t));
	if (list)
	{
		list->process_list = process_list_t_create(pool);
		list->scheduler = scheduler;
		list->alarm_manager = alarm_manager;
	}
	return list;
}

error_t proc_create_process(
		proc_list_t * const list,
		const char * image,
		const char * initial_task_name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		tinker_meminfo_t * const meminfo,
		const uint32_t flags,
		process_t ** process)
{
	error_t ret = NO_ERROR;
	process_t * proc = NULL;
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Create process with image %s, entry %x, priority %d, meminfo %x, flags %x\n",
				image, entry_point, priority, meminfo, flags);
		debug_print("Process: Create process with heap %x stack %x\n", meminfo->heap_size, meminfo->stack_size);
#endif

	if (process)
	{
		*process = 0;
	}

	/* get the new process id - SLOW! - TODO need to speed up */
	process_t * tmp = NULL;
	uint32_t proc_id = 0;
	for ( uint32_t i = 0 ; i < MAX_PROCESSES ; i++ )
	{
		if (!process_list_t_get(list->process_list, i, &tmp))
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
	const thread_t * const curr_thread = sch_get_current_thread(list->scheduler);
	mem_pool_info_t * parent_pool;
	if (curr_thread == NULL)
	{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Create process from default pool\n");
#endif
		parent_pool = mem_get_default_pool();
	} else {
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Create process for %s from parent\n", image);
#endif
		// if the parent is the kernel, use the default pool instead
		// of the kernel's
		const process_t * const parent = thread_get_parent(curr_thread);
		if (parent == kernel_get_process())
		{
		    parent_pool = mem_get_default_pool();
		}
		else
		{
		    parent_pool = process_get_mem_pool(parent);
		}
	}

#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Initialising pool for %s, heap=%x, stack=%x, parent=%x\n",
				image,
				meminfo->heap_size,
				meminfo->stack_size,
				parent_pool);
#endif
	mem_pool_info_t * new_mem_pool = NULL;
	const bool_t pool_allocated = mem_init_process_memory(
			parent_pool,
			&new_mem_pool,
			meminfo->heap_size
				+ meminfo->stack_size
				+ PAGE_TABLE_SIZE);

	if (!pool_allocated)
	{
		ret = OUT_OF_MEMORY;
	}
	else
	{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Building process: %s\n", image);
#endif
		ret = process_create(
				list->scheduler,
				list->alarm_manager,
				parent_pool,
				proc_id,
				image,
				(curr_thread == NULL),
				meminfo,
				new_mem_pool,
				&proc);

		if (ret == NO_ERROR)
		{
			object_t * process_obj = NULL;
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Building process object: %s\n", image);
#endif
			ret = obj_create_process(
					process_get_mem_pool(proc),
					process_get_object_table(proc),
					process_get_pid(proc),
					proc,
					&process_obj);
			process_set_oid(proc, obj_get_number(process_obj));

			if ( ret == NO_ERROR )
			{
				object_t * thread_obj = NULL;
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Building process entry thread: %s\n", image);
#endif
				ret = proc_create_thread(
						proc,
						initial_task_name,
						entry_point,
						priority,
						meminfo->stack_size,
						flags,
						&thread_obj,
						NULL);
				if (ret == NO_ERROR)
				{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Adding process to process list: %s\n", image);
#endif
					if (process_list_t_add(list->process_list, proc) == false)
					{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Failed to add process to process list: %s\n", image);
#endif
						process_exit(proc);
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

error_t proc_create_thread(
		process_t * process,
		const char * const name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t flags,
		object_t ** thread_object,
		thread_t ** new_thread)
{
	error_t ret = NO_ERROR;
	thread_t * thread = NULL;

	/* allocate memory for thread from processes pool */
	thread = thread_create(
			process_get_mem_pool(process),
			(const fwd_process_t*)process,
			priority,
			entry_point,
			flags,
			stack,
			name);

	/* initialise the thread */
	if (thread)
	{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Created thread %s on process %s, entry %x OK\n", name, process_get_image(process), entry_point);
#endif
		/* add the thread to the process list */
		object_number_t objno = INVALID_OBJECT_ID;
		if (process_add_thread(process, thread, &objno))
		{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Added thread %s to process %s OK\n", name, process_get_image(process));
#endif
			if (thread_get_state(thread) != THREADY_READY)
			{
#if defined (PROCESS_DEBUGGING)
		debug_print("Process: Thread %s is NOT ready\n", name);
#endif
				ret = OUT_OF_MEMORY;
			}
		}
		else
		{
			ret = OUT_OF_MEMORY;
		}

		if (ret == NO_ERROR)
		{
			if (new_thread)
			{
				*new_thread = thread;
			}

			if (thread_object)
			{
				*thread_object = obj_get_object(
						process_get_object_table(process),
						objno);
			}
		}
	}

	return ret;
}

void proc_delete_proc(
		proc_list_t * const list,
		const process_t * const process)
{
	if (list)
	{
		process_list_t_remove_item(list->process_list, (process_t*const)process);
	}
}

process_list_it_t * proc_list_procs(proc_list_t * const list)
{
	process_list_it_t * ret = NULL;
	if (list)
	{
		ret = process_list_it_t_create(list->process_list);
	}
	return ret;
}
