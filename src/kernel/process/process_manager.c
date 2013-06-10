/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "process_manager.h"

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
UNBOUNDED_LIST_BODY(extern, process_list_t, __process_t*)

/**
 * An iterator for the process list
 */
UNBOUNDED_LIST_ITERATOR_BODY(extern, process_list_it_t, process_list_t, __process_t*)

/**
 * The code to handle an unbounded list of threads for the process
 */
UNBOUNDED_LIST_BODY(extern, thread_list_t, __thread_t*)

/**
 * An iterator for the thread list
 */
UNBOUNDED_LIST_ITERATOR_BODY(extern, thread_list_it_t, thread_list_t, __thread_t*)

/**
 * The static list of processes in the system
 */
static process_list_t * __process_list = NULL;

static process_list_it_t * __process_iterator = NULL;

void __proc_initialise(void)
{
	__process_list = process_list_t_create(__mem_get_default_pool());
	__process_iterator = process_list_it_t_create(__process_list);
}

error_t __proc_create_process(
		const char * image,
		const char * initial_task_name,
		__thread_entry * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t heap,
		const uint32_t flags,
		__process_t ** process)
{
	error_t ret = NO_ERROR;
	__process_t * proc = NULL;

	/* get the new process id - SLOW! - need to speed up */
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
	__thread_t * curr_thread = __sch_get_current_thread();
	if (curr_thread == NULL )
	{
#if defined (__PROCESS_DEBUGGING )
		__debug_print("proc: create process from default pool\n");
#endif
		proc = __mem_alloc_aligned(__mem_get_default_pool(), sizeof(__process_t), MMU_PAGE_SIZE);
	} else {
#if defined (__PROCESS_DEBUGGING )
		__debug_print("proc: create process for %s from %s\n",
				image,
				curr_thread->parent->image);
#endif
		proc = __mem_alloc_aligned(curr_thread->parent->memory_pool, sizeof(__process_t), MMU_PAGE_SIZE);
	}

	if ( proc != NULL )
	{
		proc->kernel_process = (curr_thread == NULL);
		proc->process_id = proc_id;

		/* create space for the heap and stack from the parent process */
		if ( __mem_init_process_memory(&proc->memory_pool, heap + stack))
		{
			const uint32_t length = __util_strlen(image,__MAX_PROCESS_IMAGE_LEN);
			__util_memcpy(proc->image, image, length);

			proc->image[length] = '\0';

#if defined (__PROCESS_DEBUGGING )
			__debug_print("proc: initialising object table for %s\n", image);
#endif
			ret = __obj_initialse_table(proc, &proc->object_table);
			if ( ret == NO_ERROR )
			{
				__object_t * process_obj = NULL;
				__object_t * thread_obj = NULL;

#if defined (__PROCESS_DEBUGGING )
				__debug_print("proc: creating initial thread %s for %s\n",
						initial_task_name,
						image);
#endif

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
							proc->memory_pool,
							&proc->object_table,
							proc->process_id,
							&process_obj);
					proc->object_number = process_obj->object_number;

					if ( ret == NO_ERROR )
					{
						if ( process_list_t_add(__process_list, proc) == false )
						{
							__util_free(proc);
							ret = OUT_OF_MEMORY;
						}
						else
						{
							__tgt_initialise_process(proc);
						}
					}
					else
					{
						ret = OBJECT_ADD_FAILED;
					}
				}
			}
		}

		if ( process )
		{
			* process = proc;
		}
	}
	else
	{
		ret = OUT_OF_MEMORY;
	}

	return ret;
}

error_t __proc_create_thread(
		__process_t * process,
		const char * name,
		__thread_entry * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t flags,
		__object_t ** thread_object,
		__thread_t ** new_thread)
{
	error_t ret = NO_ERROR;
	__thread_t * thread = NULL;

	/* if the process doesn't contain a thread list,
	 * create it! */
	if ( process->threads == NULL )
	{
		process->threads = thread_list_t_create(process->memory_pool);
	}

	thread_list_t * list = (thread_list_t*)process->threads;
	if ( list )
	{
		if ( thread_list_t_size(list) < __MAX_THREADS )
		{
			/* get the new thread id - SLOW! - need to speed up */
			__thread_t * tmp = NULL;
			const uint32_t tmp_size = thread_list_t_size(list);
			uint32_t thread_id = 0;
			if ( tmp_size == 0 )
			{
				thread_id = 0;
			}
			else
			{
				for ( uint32_t i = 0 ; i < __MAX_THREADS ; i++ )
				{
					if ( !thread_list_t_get(list, i, &tmp) )
					{
						thread_id = i;
						break;
					}
				}
			}

			/* allocate memory for thread from processes pool */
			thread = __mem_alloc(process->memory_pool, sizeof(__thread_t));

			/* initialise the thread */
			if ( thread != NULL )
			{
				thread->parent = process;

				/* add the thread to the process list */
				thread_list_t_add(list, thread);
				const uint32_t length = __util_strlen(name,__MAX_THREAD_NAME_LEN);

				thread->thread_id = thread_id;
				thread->priority = priority;
				thread->entry_point = entry_point;
				thread->flags = flags;

				thread->stack = __mem_alloc_aligned(process->memory_pool, stack, MMU_PAGE_SIZE);
				if ( thread->stack )
				{
					bool is_kernel;
					__process_t * kernel_process = NULL;

					for ( uint32_t sp = 0 ; sp < stack ; sp++ )
					{
						*(((char*)thread->stack) + sp) = 'S';
					}
					thread->stack_size = stack;
					thread->state = thread_ready;

					kernel_process = __kernel_get_process();

					if (kernel_process)
					{
						is_kernel = (process == kernel_process);
					}
					else
					{
						/* the kernel_process is NULL therefore
						 * we're initialising the kernel process */
						is_kernel = true;
					}

					/*
					 * We need to ensure that the context information
					 * is configured properly
					 */
					/** FIXME: Change the 0 **/
					__tgt_initialise_context(thread, is_kernel, 0);
				}
				else
				{
					thread->stack_size = 0;
					thread->state = thread_not_created;
				}

				__util_memcpy(thread->name, name, length);
				thread->name[length] = '\0';

				if ( new_thread )
				{
					*new_thread = thread;
				}

				__object_t * thread_obj = NULL;
				ret = __obj_create_thread(
						process->memory_pool,
						&process->object_table,
						process->process_id,
						thread->thread_id,
						thread,
						 &thread_obj);
				if ( ret == NO_ERROR )
				{
					thread->object_number = thread_obj->object_number;
					process->thread_count++;

					if ( thread_object )
					{
						*thread_object = thread_obj;
					}
				}
			}
		}
	}
	else
	{
		ret = OUT_OF_MEMORY;
	}

	return ret;
}

void __proc_get_object_table(__object_table_t ** t)
{
	__thread_t * curr_thread = __sch_get_current_thread();
	if ( curr_thread && t )
	{
		if ( curr_thread->parent)
		{
			*t = &curr_thread->parent->object_table;
		}
	}
}

__process_t * __proc_get_process(const uint32_t process_id)
{
	__process_t * p = NULL;

	if ( process_list_t_get(__process_list, process_id, &p) == false || p == NULL )
	{
		p = NULL;
	}

	return p;
}

__thread_t * __proc_get_thread(const __process_t * proc, const uint32_t thread_id)
{
	__thread_t * t = NULL;

	if ( proc != NULL )
	{
		thread_list_t * list = ((thread_list_t*)proc->threads);
		uint32_t thread_count = thread_list_t_size(list);
		__thread_t * tmp_thread = NULL;
		for ( uint32_t tmp = 0 ; tmp < thread_count ; tmp++ )
		{
			if ( thread_list_t_get( list, thread_id, &tmp_thread) == true )
			{
				if ( tmp_thread->thread_id == thread_id )
				{
					t = tmp_thread;
					break;
				}
			}
		}
	}

	return t;
}

process_list_it_t * __proc_get_process_iterator(void)
{
	return __process_iterator;
}

process_list_it_t * __proc_get_new_process_iterator(void)
{
	return process_list_it_t_create(__process_list);
}

