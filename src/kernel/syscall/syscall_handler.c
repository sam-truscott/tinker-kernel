/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "syscall_handler.h"

#include "arch/board_support.h"
#include "sos_api_kernel_interface.h"
#include "kernel/kernel_initialise.h"
#include "kernel/kernel_main.h"
#include "arch/tgt.h"

#include "kernel/process/process_manager.h"

#include "kernel/scheduler/scheduler.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"

#include "kernel/utils/util_malloc.h"
#include "kernel/utils/util_free.h"
#include "kernel/utils/util_memcpy.h"

static inline __object_t * __syscall_get_thread_object(void)
{
	__thread_t * thread = NULL;
	__object_table_t * table = NULL;

	/* get the thread object */
	__proc_get_object_table(&table);
	thread = __sch_get_current_thread();
	return __obj_get_thread_object(table, thread->object_number);
}

void __syscall_handle_system_call(void * context, uint32_t context_size)
{
	__syscall_function_t api = (__syscall_function_t)__tgt_get_syscall_param(context, 0);
	uint32_t ret = 0;
	/* FIXME hard coded 7 */
	uint32_t param[7];
	param[0] = __tgt_get_syscall_param(context, 1);
	param[1] = __tgt_get_syscall_param(context, 2);
	param[2] = __tgt_get_syscall_param(context, 3);
	param[3] = __tgt_get_syscall_param(context, 4);
	param[4] = __tgt_get_syscall_param(context, 5);
	param[5] = __tgt_get_syscall_param(context, 6);
	param[6] = __tgt_get_syscall_param(context, 7);
	__thread_t * this_thread = __sch_get_current_thread();

	/* TODO FIXME This is accounting for things being passed on the
	 * stack which'll have a different base address as they'll be
	 * at some weird virtual address */
	for ( uint8_t i = 0 ; i < 7 ; i++ )
	{
		if ( param[i] >= 0xC0000000u )
		{
			param[i] -= 0xC0000000u;
			param[i] += this_thread->r_stack_base - 1428;
			//param[i] = this_thread->v_stack_base - param[i];
			//param[i] += this_thread->r_stack_base;
		}
	}

	/*
	 * This could use a jump table but I think in this
	 * case this is a bit more readable.
	 */
	switch ( api )
	{
		case syscall_create_process:
			{
				__process_t * process;
				ret = __proc_create_process(
						(const char *)param[0],
						"main",
						(__thread_entry *)(param[1]),
						(const uint8_t)param[2],
						(const uint32_t)param[3],
						(const uint32_t)param[4],
						(uint32_t)param[5],
						(__process_t **)&process);
				*((object_number_t*)param[6]) = process->object_number;
			}
			break;

		case syscall_create_thread:
			{
				__process_t * process;
				__thread_t * thread;

				process = this_thread->parent;

				ret = __proc_create_thread(
						process,
						(const char*)param[0],
						(__thread_entry * )(param[1]),
						(const uint8_t)param[2],
						(const uint32_t)param[3],
						(uint32_t)param[4],
						NULL,
						(__thread_t **)&thread);
				*((object_number_t*)param[5]) = thread->object_number;
			}
			break;

		case syscall_thread_priority:
			{
				object_number_t thread_no = (object_number_t)param[0];
				priority_t * priority = (priority_t*)param[1];
				if ( thread_no && priority)
				{
					__object_table_t * table = NULL;
					__object_t * thread_obj = NULL;

					__proc_get_object_table(&table);

					thread_obj = __obj_get_thread_object(table, thread_no);
					if ( thread_obj )
					{
						*priority = thread_obj->specifics.thread.thread->priority;
						ret = NO_ERROR;
					}
					else
					{
						ret = INVALID_OBJECT;
					}
				}
				else
				{
					ret = INVALID_OBJECT;
				}
			}
			break;

		case syscall_thread_object:
			{
				__object_t * thread_obj = __syscall_get_thread_object();
				if ( thread_obj )
				{
					*((object_number_t*)param[0]) = thread_obj->object_number;
				}
			}
			break;

		case syscall_exit_thread:

		{
			__object_t * thread_obj =__syscall_get_thread_object();
			if ( thread_obj )
			{
				ret = __obj_exit_thread(thread_obj);
			}
		}
			break;

		case syscall_malloc:
            if ( param[0] && !param[1] && !param[2] && !param[3] && !param[4] && !param[5])
            {
            	ret = (uint32_t)__util_malloc(param[0]);
            }
			break;

		case syscall_mfree:
			if ( param[0] )
			{
				__util_free((void *)param[0]);
			}
			break;

		case syscall_create_semaphore:
			if ( param[0] )
			{
				__object_table_t * table = NULL;
				__process_t * proc = NULL;

				if ( this_thread )
				{
					proc = this_thread->parent;
				}

				__proc_get_object_table(&table);

				ret = __obj_create_semaphore(
						proc->memory_pool,
						table,
						(__object_t**)param[0],
						(const uint32_t)param[1]);
			}
			break;

		case syscall_get_semaphore:
		{
			__object_t * thread_obj = __syscall_get_thread_object();
			if ( thread_obj )
			{
				ret = __obj_get_semaphore( thread_obj, (__object_t*)param[0]);
			}
			else
			{
				ret = INVALID_OBJECT;
			}
			/* TODO check the return vector if the current thread isn't right */
			break;
		}
		case syscall_release_semaphore:
		{
			__object_t * thread_obj = __syscall_get_thread_object();
			if ( thread_obj )
			{
				ret = __obj_release_semaphore( thread_obj, (__object_t*)param[0] );
			}
			break;
		}

		case syscall_load_thread:

			if ( __kernel_is_first_run() )
			{
				__kernel_first_run_ok();
			}
			else
			{
				/* save the schedulers (where we've come from) context
				 * for re-entry next time */
				__util_memcpy(
						__kernel_get_scheduler_thread()->context,
						context,
						context_size);
			}
			__tgt_prepare_context(context, this_thread);
			break;

		default:
			/* setup the error_number and add associated error number utilities */
			ret = syscall_unknown;
			break;
	}

	/* This will over-ride the result of a system-call
	 * if the exception occurs just after the system call has been made. */
	if ( api != syscall_load_thread )
	{
		__tgt_set_syscall_return(context, ret);
	}
	else
	{
		__bsp_enable_schedule_timer();
	}

	/*
	 * If the thread has been un-scheduled we need to switch process
	 */
	if ( (this_thread->state != thread_system) &&
		 (this_thread->state != thread_running) )
	{
		/* save the existing data - i.e. the return & run the scheduler */
		__sch_save_context(context, context_size);
		__sch_prepare_scheduler_context(context, context_size);
	}
}
