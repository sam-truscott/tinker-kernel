/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "scheduler.h"

#include "arch/tgt.h"
#include "arch/board_support.h"
#include "kernel/kernel_assert.h"
#include "kernel/kernel_initialise.h"
#include "kernel/utils/util_memcpy.h"

#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_semaphore.h"

/**
 * The scheduler implementation we're using
 */
static __scheduler_t __sch_scheduler_implementation;

void __sch_initialise(__scheduler_t * impl)
{
	__util_memcpy(&__sch_scheduler_implementation, impl, sizeof(__scheduler_t));
}

void __sch_initialise_scheduler(void)
{
	if ( __sch_scheduler_implementation.initialise )
	{
		__sch_scheduler_implementation.initialise();
	}
}

void __sch_notify_new_thread(__thread_t * t)
{
	if ( __sch_scheduler_implementation.new_thread )
	{
		__sch_scheduler_implementation.new_thread(t);
	}
}

void __sch_notify_exit_thread(__thread_t * t)
{
	if ( __sch_scheduler_implementation.exit_thread )
	{
		__sch_scheduler_implementation.exit_thread(t);
	}
}

void __sch_notify_pause_thread(__thread_t * t)
{
	if ( __sch_scheduler_implementation.pause_thread )
	{
		__sch_scheduler_implementation.pause_thread(t);
	}
}

void __sch_notify_resume_thread(__thread_t * t)
{
	if ( __sch_scheduler_implementation.resume_thread )
	{
		__sch_scheduler_implementation.resume_thread(t);
	}
}

void __sch_notify_change_priority(__thread_t * t, priority_t original_priority)
{
	if ( __sch_scheduler_implementation.change_priority )
	{
		__sch_scheduler_implementation.change_priority(t, original_priority);
	}
}

void __sch_terminate_current_thread(void * context, uint32_t context_size)
{
	__thread_t * thread = __sch_get_current_thread();

	if ( thread )
	{
		thread->state = thread_terminated;
		__util_memcpy(thread->context, context, context_size);
		__sch_notify_exit_thread(thread);
	}
}

void __sch_scheduler(void)
{
	while(1==1)
	{
		__thread_t * next_thread = NULL;

		/* Setup the next set of vectors */
		__kernel_assert("No scheduler implementation installed!",
				__sch_scheduler_implementation.scheduler != NULL);

		__sch_scheduler_implementation.scheduler(&next_thread);
		if ( next_thread == NULL )
		{
			next_thread = __kernel_get_idle_thread();
		}

		__kernel_assert("Scheduler couldn't get next thread", next_thread != NULL);

		SOS_API_CALL_0(syscall_load_thread);
	}
}

__thread_t * __sch_get_current_thread(void)
{
	__thread_t * t = NULL;
	if ( __sch_scheduler_implementation.get_curr_thread )
	{
		t = __sch_scheduler_implementation.get_curr_thread();
	}
	return t;
}

void __sch_set_current_thread(__thread_t * thread)
{
	__sch_scheduler_implementation.set_curr_thread(thread);
}

void __sch_prepare_scheduler_context(
		void * context,
		uint32_t context_size)
{
	__util_memcpy(context, __kernel_get_scheduler_thread()->context, context_size);
}

void __sch_save_context(
		void * context,
		uint32_t context_size)
{
	__thread_t * curr_thread = __sch_get_current_thread();
	if ( curr_thread )
	{
		/*
		 * Move the thread from the running state to the ready state
		 */
		if ( curr_thread->state == thread_running )
		{
			curr_thread->state = thread_ready;
		}
		__util_memcpy(curr_thread->context, context, context_size);
	}
}
