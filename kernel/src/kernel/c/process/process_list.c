/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "process/process_list.h"
#include "process/process_list_private.h"

#include "config.h"
#include "tgt.h"
#include "utils/util_strlen.h"
#include "objects/object_table.h"
#include "objects/obj_process.h"
#include "objects/obj_thread.h"
#include "memory/memory_manager.h"
//#include "memory/mem_section.h"
#include "console/print_out.h"

#define INVALID_PROC_ID 0

UNBOUNDED_LIST_ITERATOR_INTERNAL_TYPE(process_list_it_t, process_list_t, process_t*)
UNBOUNDED_LIST_ITERATOR_BODY(extern, process_list_it_t, process_list_t, process_t*)

proc_list_t * proc_create(
		mem_pool_info_t * const pool,
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager)
{
	proc_list_t * const list = mem_alloc(pool, sizeof(process_list_t));
	if (list)
	{
		util_memset(list, 0, sizeof(proc_list_t));
		list->process_list = process_list_t_create(pool);
		list->scheduler = scheduler;
		list->alarm_manager = alarm_manager;
		list->last_pid = 1;
		list->kernel_process = NULL;
	}
	return list;
}

void proc_set_kernel_process(
		proc_list_t * const list,
		process_t * const kernel_process)
{
	if (list)
	{
		list->kernel_process = kernel_process;
	}
}

process_t * proc_get_kernel_process(proc_list_t * const list)
{
	return list == NULL ? NULL : list->kernel_process;
}

static inline mem_pool_info_t * get_parent_pool(
		proc_list_t * const list,
		const char * const image,
		const thread_t * const curr_thread)
{
	/*
	 * If the current thread is null we're in initialisation and
	 * it means we're creating the kernels process so we can
	 * use the default pool.
	 *
	 * Otherwise we need to use the current processes pool.
	 */
	mem_pool_info_t * parent_pool;
	if (curr_thread == NULL)
	{
		debug_prints(PROCESS, "Process: Create process from default pool\n");
		parent_pool = mem_get_default_pool();
	}
	else
	{
		(void)image;
		debug_print(PROCESS, "Process: Create process for %s from parent\n", image);
		// if the parent is the kernel, use the default pool instead
		// of the kernel's
		const process_t * const parent = thread_get_parent(curr_thread);
		if (list->kernel_process == NULL || parent == list->kernel_process)
		{
			parent_pool = mem_get_default_pool();
		}
		else
		{
			parent_pool = process_get_mem_pool(parent);
		}
	}
	return parent_pool;
}

static inline uint32_t get_proc_id(proc_list_t * const list)
{
	/* get the new process id - TODO need to randomise */
	process_t * tmp = NULL;
	uint32_t proc_id = INVALID_PROC_ID;
	for (uint32_t i = list->last_pid ; i < MAX_PROCESSES ; i++)
	{
		if (!process_list_t_get(list->process_list, i, &tmp))
		{
			proc_id = i;
			list->last_pid = i;
			break;
		}
	}
	return proc_id;
}

static inline bool_t initialise_mem_pool(
		const char * const image,
		mem_pool_info_t * const parent_pool,
		tinker_meminfo_t * const meminfo,
		mem_pool_info_t ** new_mem_pool)
{
	(void)image;
	debug_print(PROCESS,
			"Process: Initialising pool for %s, heap=%x, stack=%x, parent=%x\n",
			image,
			meminfo->heap_size,
			meminfo->stack_size,
			parent_pool);
	return mem_init_process_memory(
			parent_pool,
			new_mem_pool,
			meminfo->heap_size
				+ meminfo->stack_size
				+ PAGE_TABLE_SIZE
				+ PRIVATE_POOL_SIZE);
}

static inline return_t create_process(
		proc_list_t * const list,
		mem_pool_info_t * const parent_pool,
		const uint32_t proc_id,
		const char * const image,
		const bool_t is_kernel,
		tinker_meminfo_t * const meminfo,
		mem_pool_info_t * const new_mem_pool,
		const mem_section_t * const ksection,
		process_t ** proc)
{
	debug_print(PROCESS, "Process: Building process: %s\n", image);
	return process_create(
			list->scheduler,
			list->alarm_manager,
			parent_pool,
			proc_id,
			image,
			is_kernel,
			meminfo,
			new_mem_pool,
			ksection,
			proc);
}

static inline return_t create_process_object(
		const char * const image,
		proc_list_t * const list,
		process_t * const proc,
		object_t * process_obj)
{
	(void)image;
	debug_print(PROCESS, "Process: Building process object: %s\n", image);
	return_t ret = obj_create_process(
			list,
			process_get_mem_pool(proc),
			process_get_object_table(proc),
			process_get_pid(proc),
			proc,
			&process_obj);
	process_set_oid(proc, obj_get_number(process_obj));
	return ret;
}

static inline return_t create_thread_object(
		const char * const image,
		process_t * const proc,
		const char * const initial_task_name,
		thread_entry_point * const entry_point,
		const uint8_t priority,
		const uint32_t stack_size,
		const uint32_t flags,
		object_t ** thread_obj)
{
	(void)image;
	debug_print(PROCESS, "Process: Building process entry thread: %s (stack %8x)\n", image, stack_size);
	return proc_create_thread(
			proc,
			initial_task_name,
			entry_point,
			priority,
			stack_size,
			flags,
			thread_obj,
			NULL);
}

static inline bool_t add_process_to_list(
		const char * const image,
		proc_list_t * const list,
		process_t * const proc)
{
	(void)image;
	debug_print(PROCESS, "Process: Adding process to process list: %s\n", image);
	bool_t ok = (process_list_t_add(list->process_list, proc));
	debug_print(PROCESS, "Process: Add process to process list: %s = %d\n", image, ok);
	return ok;
}

return_t proc_create_process(
		proc_list_t * const list,
		const char * image,
		const char * initial_task_name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		tinker_meminfo_t * const meminfo,
		const uint32_t flags,
		process_t ** process)
{
	return_t ret = NO_ERROR;
	process_t * proc = NULL;
	debug_print(PROCESS, "Process: Create process with image %s, entry %x, priority %d, meminfo %x, flags %x\n",
			image, entry_point, priority, meminfo, flags);
	debug_print(PROCESS, "Process: Create process with heap %x stack %x\n", meminfo->heap_size, meminfo->stack_size);

	if (process)
	{
		*process = NULL;
	}

	const uint32_t proc_id = get_proc_id(list);
	if (proc_id == INVALID_PROC_ID)
	{
		ret = OUT_OF_PROCESS_IDS;
	}
	bool_t pool_allocated = false;
	mem_pool_info_t * new_mem_pool = NULL;
	mem_pool_info_t * parent_pool = NULL;
	object_t * thread_obj = NULL;
	if (ret == NO_ERROR)
	{
		const thread_t * const curr_thread = sch_get_current_thread(list->scheduler);
		parent_pool = get_parent_pool(list, image, curr_thread);

		pool_allocated = initialise_mem_pool(image, parent_pool, meminfo, &new_mem_pool);
		if (!pool_allocated)
		{
			ret = OUT_OF_MEMORY;
		}
		else
		{
			ret = create_process(
					list,
					parent_pool,
					proc_id,
					image,
					(curr_thread == NULL),
					meminfo,
					new_mem_pool,
					process_get_first_section(list->kernel_process),
					&proc);
			if (ret == NO_ERROR)
			{
				object_t * process_obj = NULL;
				ret = create_process_object(image, list, proc, process_obj);

				if (ret == NO_ERROR)
				{
					ret = create_thread_object(
							image,
							proc,
							initial_task_name,
							entry_point,
							priority,
							meminfo->stack_size,
							flags,
							&thread_obj);
					if (ret == NO_ERROR)
					{
						if (add_process_to_list(image, list, proc))
						{
							if (process)
							{
								(*process) = proc;
							}
						}
						else
						{
							ret = OUT_OF_MEMORY;
						}
					}
					else
					{
						ret = OBJECT_ADD_FAILED;
					}
				}
			}
		}
	}

	if (ret != NO_ERROR)
	{
		// will delete any objects remaining in the table
		if (proc)
		{
			process_exit(proc);
		}
		if (pool_allocated)
		{
			mem_free(parent_pool, new_mem_pool);
		}
	}

	return ret;
}

return_t proc_create_thread(
		process_t * process,
		const char * const name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t flags,
		object_t ** thread_object,
		thread_t ** new_thread)
{
	return_t ret = NO_ERROR;
	thread_t * thread = NULL;

	/* allocate memory for thread from processes pool */
	debug_print(PROCESS, "Process: Creating thread %s stack %8x\n", name, stack);
	thread = thread_create(
			process_get_mem_pool(process),
			process_get_user_mem_pool(process),
			(const fwd_process_t*)process,
			priority,
			entry_point,
			flags,
			stack,
			name);

	/* initialise the thread */
	if (thread)
	{
		debug_print(PROCESS, "Process: Created thread %s on process %s, entry %x OK\n", name, process_get_image(process), entry_point);
		/* add the thread to the process list */
		object_number_t objno = INVALID_OBJECT_ID;
		if (process_add_thread(process, thread, &objno))
		{
		debug_print(PROCESS, "Process: Added thread %s to process %s OK\n", name, process_get_image(process));
			if (thread_get_state(thread) != THREAD_READY)
			{
				debug_print(PROCESS, "Process: Thread %s is NOT ready\n", name);
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
	} else {
		error_print("Failed to allocate memory of thread %s", name);
	}

	return ret;
}

void proc_delete_proc(
		proc_list_t * const list,
		process_t * const process)
{
	if (list)
	{
		process_list_t_remove_item(list->process_list, process);
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
