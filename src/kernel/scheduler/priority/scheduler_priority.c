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

#include "scheduler_priority.h"

#include "kernel/kernel_initialise.h"
#include "kernel/kernel_panic.h"
#include "kernel/process/process_manager.h"
#include "kernel/utils/collections/unbounded_queue.h"
#include "kernel/utils/collections/stack.h"

UNBOUNDED_QUEUE_SPEC(static, thread_queue_t, __thread_t*)
UNBOUNDED_QUEUE_BODY(static, thread_queue_t, __thread_t*)

STACK_SPEC(static, queue_stack_t, thread_queue_t* )
STACK_BODY(static, queue_stack_t, thread_queue_t* )

/**
 * The current process being used
 */
static process_list_it_t * __sch_process_iterator = NULL;

static thread_list_it_t * __sch_thread_iterator = NULL;

/**
 * The current thread executing
 */
static __thread_t * __sch_current_thread = NULL;

static thread_queue_t * __sch_active_queue;

static thread_queue_t __sch_thread_queues[__MAX_PRIORITY + 1];

static uint8_t __sch_current_priority;

static queue_stack_t __sch_queue_stack;

static void __sch_priority_find_next_queue(__thread_t * t);

static bool __sch_initialised = false;

void __sch_priority_initialise(void)
{
	if ( !__sch_initialised )
	{
		__process_t * proc = NULL;

		__sch_current_priority = 0;

		for ( uint16_t i = 0 ; i < __MAX_PRIORITY + 1 ; i++ )
		{
			thread_queue_t_initialise(&__sch_thread_queues[i], __mem_get_default_pool());
		}

		__sch_active_queue = &(__sch_thread_queues[0]);
		queue_stack_t_initialise( &__sch_queue_stack, __mem_get_default_pool() );
		queue_stack_t_push(&__sch_queue_stack, __sch_active_queue);

		/* __sch_priority_stack = queue_stack_t_create(__kernel_get_process()->heap_stack); */

		/* get the process iterator */
		__sch_process_iterator = __proc_get_process_iterator();
		process_list_it_t_get(__sch_process_iterator, &proc);

		/* get the thread iterator and first thread */
		__sch_thread_iterator = thread_list_it_t_create(proc->threads);
		thread_list_it_t_get(__sch_thread_iterator, &__sch_current_thread);

		__sch_initialised = true;
	}
}

void __sch_priority_execute(__thread_t ** new_thread)
{
	if ( __sch_active_queue )
	{
		thread_queue_t_front(__sch_active_queue, &__sch_current_thread);
		thread_queue_t_pop(__sch_active_queue);
	}

	/* once we're either back at the start or we've
	 * selected a new thread check its running and
	 * then use it */
	if ( new_thread && __sch_current_thread )
	{
		thread_queue_t_push(__sch_active_queue, __sch_current_thread);
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

void __sch_priority_notify_new_thread(__thread_t * t)
{
	if ( !__sch_initialised )
	{
		__sch_priority_initialise();
	}

	if ( t )
	{
		thread_queue_t * queue = &(__sch_thread_queues[t->priority]);
		thread_queue_t_push(queue, t);

		if ( t->priority > __sch_current_priority )
		{
				__sch_current_priority = t->priority;
				__sch_active_queue = &(__sch_thread_queues[__sch_current_priority]);
				queue_stack_t_push(&__sch_queue_stack, __sch_active_queue);
		}
		else if ( t->priority < __sch_current_priority )
		{
			/* if we add a thread that's of a lower priority then itmust be added
			 * to the queue stack so when the new high priority task finishes the
			 *  new lower priority task won't be executed */
			thread_queue_t * stack_queue = NULL;
			const uint32_t stack_size = queue_stack_t_size(&__sch_queue_stack);
			int32_t current_priority = (int32_t)(stack_size - 1);
			bool found = false;
			int32_t insert_index = -1;

			/*
			 * the stack queue uses the end of the queue (size) as the highest priority
			 * and the start (0) of the queue as the lowest priority
			 */
			while(current_priority >= 0)
			{
				if ( queue_stack_t_get(&__sch_queue_stack, current_priority, &stack_queue) )
				{
					const uint32_t queue_size = thread_queue_t_size(stack_queue);
					if ( queue_size )
					{
						__thread_t * nt = NULL;
						bool ok;
						ok = thread_queue_t_front(stack_queue, &nt);
						if ( ok == true )
						{
							if ( nt->priority == t->priority )
							{
								/* it's already on the stack */
								found = true;
								break;
							}
							else if ( nt->priority < t->priority)
							{
								insert_index = current_priority + 1;
								found = false;
								break;
							}
						}
					}
				}
				current_priority--;
			}

			/*
			 * we meed to insert the queue into the stack to ensure
			 * that at somepoint it'll be scheduled
			 */
			if ( !found && insert_index >= 0 )
			{
				if (!queue_stack_t_insert(&__sch_queue_stack, insert_index, queue))
				{
					/* failed to add it to the scheduler, panic */
					__kernel_panic();
				}
			}
		}
	}
}

void __sch_priority_notify_exit_thread(__thread_t * t)
{
	if ( t )
	{
		thread_queue_t * queue = &(__sch_thread_queues[t->priority]);
		thread_queue_t_remove(queue, t);

		if ( thread_queue_t_size(__sch_active_queue) == 0 )
		{
			/* pop the one we're using off */
			queue_stack_t_pop(&__sch_queue_stack, &__sch_active_queue);
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if ( !queue_stack_t_front(&__sch_queue_stack, &__sch_active_queue) )
			{
				__sch_priority_find_next_queue(t);
			}
		}
	}
}

void __sch_priority_notify_pause_thread(__thread_t * t)
{
	__sch_priority_notify_exit_thread(t);
}

void __sch_priority_notify_resume_thread(__thread_t * t)
{
	__sch_priority_notify_new_thread(t);
}

void __sch_priority_notify_change_priority(__thread_t * t, priority_t original_priority)
{
	if ( t )
	{
		/* remove it from the old list */
		thread_queue_t * queue = &(__sch_thread_queues[original_priority]);
		thread_queue_t_remove(queue, t);

		/* add it to the new list */
		queue = &(__sch_thread_queues[t->priority]);
		thread_queue_t_push(queue, t);

		if ( thread_queue_t_size(__sch_active_queue) == 0 )
		{
			__sch_current_priority = t->priority;

			/* pop the one we're using off */
			queue_stack_t_pop(&__sch_queue_stack, &__sch_active_queue);
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if ( !queue_stack_t_front(&__sch_queue_stack, &__sch_active_queue) )
			{
				__sch_priority_find_next_queue(t);
			}
		}
	}
}

__thread_t * __sch_priority_get_curr_thread(void)
{
	return __sch_current_thread;
}

void __sch_priority_set_curr_thread(__thread_t * thread)
{
	if ( thread )
	{
		__sch_current_thread = thread;
	}
}

static void __sch_priority_find_next_queue(__thread_t * t)
{
	uint32_t size;
	priority_t p = t->priority;
	thread_queue_t * queue;

	/* there will always be 1 - the idle thread */
	do
	{
		queue = &(__sch_thread_queues[--p]);
		size = thread_queue_t_size(queue);
		if ( size )
		{
			break;
		}
	} while ( !size && p );

	__sch_current_priority = p;
	__sch_active_queue = queue;
	queue_stack_t_push(&__sch_queue_stack, __sch_active_queue);
}
