/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "scheduler_rr.h"
#include "kernel/process/process_manager.h"
#include "kernel/debug/debug_print.h"

/**
 * The current process being used
 */
static process_list_it_t * __sch_process_iterator = NULL;

static thread_list_it_t * __sch_thread_iterator = NULL;

/**
 * The current thread executing
 */
static __thread_t * __sch_current_thread = NULL;

__thread_t * __sch_rr_get_curr_thread(void)
{
	return __sch_current_thread;
}

void __sch_rr_set_curr_thread(__thread_t * thread)
{
	if ( thread )
	{
		__sch_current_thread = thread;
	}
}

void __sc_rr_initialise(void)
{
	__process_t * proc = NULL;
	/* get the process iterator */
	__sch_process_iterator = __proc_get_process_iterator();
	process_list_it_t_get(__sch_process_iterator, &proc);

	/* get the thread iterator and first thread */
	__sch_thread_iterator = thread_list_it_t_create(proc->threads);
	thread_list_it_t_get(__sch_thread_iterator, &__sch_current_thread);
}

void __sch_rr_execute(__thread_t ** new_thread)
{
	__thread_t * next = NULL;
	const __thread_t * current_thread = __sch_current_thread;

	while ( next != current_thread)
	{
		/* first, attempt to get the next process in this list */
		if ( !thread_list_it_t_next(__sch_thread_iterator, &next) )
		{
			__process_t * proc = NULL;
			/* if the next thread doesn't exist, get the next process */
			if ( process_list_it_t_next(__sch_process_iterator, &proc) )
			{
				thread_list_it_t_initialise(__sch_thread_iterator, proc->threads);
				thread_list_it_t_get(__sch_thread_iterator, &next);
			}
			else
			{
				/* rewind to the start */
				process_list_it_t_reset(__sch_process_iterator);
				process_list_it_t_get(__sch_process_iterator, &proc);

				/* get the thread iterator and first thread */
				thread_list_it_t_initialise(__sch_thread_iterator, proc->threads);
				thread_list_it_t_get(__sch_thread_iterator, &next);
			}
		}
		if ( next )
		{
			if ( next->state == thread_ready)
			{
				__sch_current_thread = next;
				break;
			}
		}
	}

	/* once we're either back at the start or we've
	 * selected a new thread check its running and
	 * then use it */
	if ( new_thread )
	{
		if ( __sch_current_thread->state == thread_ready)
		{
			 __sch_current_thread->state = thread_running;
			*new_thread = __sch_current_thread;
		}
		else
		{
			*new_thread = NULL;
		}
	}
}

