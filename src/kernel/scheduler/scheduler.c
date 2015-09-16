/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "scheduler.h"
#include "scheduler_private.h"

#include "arch/tgt.h"
#include "arch/board_support.h"

#include "kernel/kernel_assert.h"
#include "kernel/kernel_panic.h"
#include "kernel/process/process_list.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/utils/util_memcpy.h"


static void sch_priority_find_next_queue(scheduler_t * const scheduler, thread_t * const t);

scheduler_t * sch_create_scheduler(mem_pool_info_t * const pool)
{
	scheduler_t * sch = mem_alloc(pool, sizeof(scheduler_t));
	if (sch)
	{
		util_memset(sch, 0, sizeof(scheduler_t));
		for (uint16_t i = 0 ; i < MAX_PRIORITY + 1 ; i++)
		{
			thread_queue_t_initialise(&sch->priority_queues[i], mem_get_default_pool());
		}

		sch->curr_queue = &(sch->priority_queues[0]);
		queue_stack_t_initialise( &sch->queue_stack, mem_get_default_pool() );
		queue_stack_t_push(&sch->queue_stack, sch->curr_queue);
	}
	return sch;
}

void sch_set_kernel_idle_thread(scheduler_t * const sch, thread_t * const idle_thread)
{
	if (sch)
	{
		sch->idle_thread = idle_thread;
	}
}

static void insert_lower_priority_queue_to_stack(
		scheduler_t * const scheduler,
		const priority_t thread_priority,
		thread_queue_t * const queue)
{
	/* if we add a thread that's of a lower priority then it must be added
	 * to the queue stack so when the new high priority task finishes the
	 *  new lower priority task will be executed */
	thread_queue_t * stack_queue = NULL;
	const uint32_t stack_size = queue_stack_t_size(&scheduler->queue_stack);
	int32_t stack_index = (int32_t)(stack_size - 1);
	bool_t found = false;
	int32_t insert_index = -1;

	/*
	 * the stack queue uses the end of the queue (size) as the highest priority
	 * and the start (0) of the queue as the lowest priority
	 *
	 * --> the below code works out where to install the priority queue into
	 * --> the stack which is priority based
	 *
	 * FIXME: This is a crap linear search.
	 *
	 */
	while (stack_index >= 0 && !found && insert_index == -1)
	{
		if (queue_stack_t_get(&scheduler->queue_stack, stack_index, &stack_queue))
		{
			const uint32_t queue_size = thread_queue_t_size(stack_queue);
			if (queue_size)
			{
				thread_t * firstThreadInQueue = NULL;
				const bool_t ok = thread_queue_t_front(stack_queue, &firstThreadInQueue);
				if (ok)
				{
					const priority_t stack_thread_priority = thread_get_priority(firstThreadInQueue);
					if ( stack_thread_priority == thread_priority )
					{
						/* it's already on the stack */
						found = true;
					}
					else if ( stack_thread_priority < thread_priority)
					{
						insert_index = stack_index + 1;
					}
				}
			}
		}
		stack_index--;
	}

	/*
	 * we need to insert the queue into the stack to ensure
	 * that at some point it'll be scheduled
	 */
	if ( !found && insert_index >= 0 )
	{
		if (!queue_stack_t_insert(&scheduler->queue_stack, insert_index, queue))
		{
			/* failed to add it to the scheduler, panic */
			kernel_panic();
		}
	}
}

void sch_notify_new_thread(scheduler_t * const scheduler, thread_t * const t)
{
	if (scheduler && t)
	{
		const priority_t thread_priority = thread_get_priority(t);
		thread_queue_t * const queue = &(scheduler->priority_queues[thread_priority]);
		thread_queue_t_push(queue, t);

#if defined(SCHEDULER_DEBUGGING)
		debug_print("Scheduler: New thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
#endif

		if (thread_priority > scheduler->curr_priority)
		{
			scheduler->curr_priority = thread_priority;
			scheduler->curr_queue = &(scheduler->priority_queues[scheduler->curr_priority]);
			queue_stack_t_push(&scheduler->queue_stack, scheduler->curr_queue);
		}
		else if ( thread_priority < scheduler->curr_priority )
		{
			insert_lower_priority_queue_to_stack(scheduler, thread_priority, queue);
		}
	}
#if defined(SCHEDULER_DEBUGGING)
		debug_print("Scheduler: New priority, now (%d)\n", scheduler->curr_priority);
#endif
}

void sch_notify_exit_thread(scheduler_t * const scheduler, thread_t * const t)
{
	if (t)
	{
		const priority_t thread_priority = thread_get_priority(t);
		thread_queue_t * const queue = &(scheduler->priority_queues[thread_priority]);
		const bool_t removed = thread_queue_t_remove(queue, t);

#if defined(SCHEDULER_DEBUGGING)
		debug_print("Scheduler: Exit thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
#endif
		if (removed && thread_queue_t_size(scheduler->curr_queue) == 0)
		{
			/* pop the one we're using off */
			queue_stack_t_pop(&scheduler->queue_stack, &scheduler->curr_queue);
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if (queue_stack_t_front(&scheduler->queue_stack, &scheduler->curr_queue))
			{
				thread_t * first_thread = NULL;
				if (thread_queue_t_front(scheduler->curr_queue, &first_thread))
				{
					scheduler->curr_priority = thread_get_priority(first_thread);
				}
				else
				{
					scheduler->curr_priority = 0;
				}
			}
			else
			{
				sch_priority_find_next_queue(scheduler, t);
			}
		}
	}
#if defined(SCHEDULER_DEBUGGING)
		debug_print("Scheduler: Exit priority, now (%d)\n", scheduler->curr_priority);
#endif
}

void sch_notify_pause_thread(scheduler_t * const scheduler, thread_t * const t)
{
	sch_notify_exit_thread(scheduler, t);
}

void sch_notify_resume_thread(scheduler_t * const scheduler, thread_t * const t)
{
	sch_notify_new_thread(scheduler, t);
}

void sch_notify_change_priority(
		scheduler_t * const scheduler,
		thread_t * const t,
		const priority_t original_priority)
{
	if (t)
	{
		const priority_t thread_priority = thread_get_priority(t);

#if defined(SCHEDULER_DEBUGGING)
		debug_print("Scheduler: Change thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
#endif

		/* remove it from the old list */
		thread_queue_t * queue = &(scheduler->priority_queues[original_priority]);
		thread_queue_t_remove(queue, t);

		/* add it to the new list */
		queue = &(scheduler->priority_queues[thread_priority]);
		thread_queue_t_push(queue, t);

		// handle an empty priority queue - if it's empty remove it
		const bool_t is_curr_queue_empty = !thread_queue_t_size(scheduler->curr_queue);
		if (is_curr_queue_empty)
		{
			/* pop the one we're using off */
			queue_stack_t_pop(&scheduler->queue_stack, &scheduler->curr_queue);
		}
		if (thread_priority > original_priority)
		{
			scheduler->curr_priority = thread_priority;
			scheduler->curr_queue = queue;
			queue_stack_t_push(&scheduler->queue_stack, queue);
		}
		else
		{
			insert_lower_priority_queue_to_stack(scheduler, thread_priority, queue);
		}
		if (is_curr_queue_empty)
		{
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if (queue_stack_t_front(&scheduler->queue_stack, &scheduler->curr_queue))
			{
				thread_t * new_thread = NULL;
				if (thread_queue_t_front(scheduler->curr_queue, &new_thread))
				{
					scheduler->curr_priority = thread_get_priority(new_thread);
				}
				else
				{
					kernel_panic();
				}
			}
			else
			{
				sch_priority_find_next_queue(scheduler, t);
			}
		}
	}
#if defined(SCHEDULER_DEBUGGING)
		debug_print("Scheduler: Change priority, now (%d)\n", scheduler->curr_priority);
#endif
}

void sch_terminate_current_thread(
		scheduler_t * const scheduler,
		const tgt_context_t * const context)
{
	thread_t * const thread = sch_get_current_thread(scheduler);
	kernel_assert("Attempted to terminate idle thread\n", thread!=scheduler->idle_thread);
	if (thread)
	{
		thread_set_state(thread, THREAD_TERMINATED);
		thread_save_context(thread, context);
		sch_notify_exit_thread(scheduler, thread);
	}
}

thread_t * sch_get_current_thread(scheduler_t * const scheduler)
{
	return scheduler->curr_thread;
}

void sch_set_current_thread(scheduler_t * const scheduler, thread_t * const thread)
{
	if (thread)
	{
		scheduler->curr_thread = thread;
	}
}

static void sch_priority_find_next_queue(scheduler_t * const scheduler, thread_t * const t)
{
	uint32_t size;
	priority_t p = thread_get_priority(t);
	thread_queue_t * queue;

	/* there will always be 1 - the idle thread */
	do
	{
		queue = &(scheduler->priority_queues[--p]);
		size = thread_queue_t_size(queue);
		if ( size )
		{
			break;
		}
	} while (!size && p);

	scheduler->curr_priority = p;
	scheduler->curr_queue = queue;
	queue_stack_t_push(&scheduler->queue_stack, scheduler->curr_queue);
}

void sch_set_context_for_next_thread(
		scheduler_t * const scheduler,
		tgt_context_t * const context,
		const thread_state_t thread_state)
{
	thread_t * const current_thread = scheduler->curr_thread;
	if (scheduler->curr_queue)
	{
		thread_queue_t_front(scheduler->curr_queue, &scheduler->curr_thread);
	}

	/* once we're either back at the start or we've
	 * selected a new thread check its running and
	 * then use it */
	if (scheduler->curr_thread)
	{
		const bool_t reorder_ok = thread_queue_t_reorder_first(scheduler->curr_queue);
		kernel_assert("re-ordering of priority queue failed", reorder_ok);
		const thread_state_t state = thread_get_state(scheduler->curr_thread);

		if (state == THREAD_READY || state == THREAD_SYSTEM)
		{
			thread_set_state(scheduler->curr_thread, THREAD_RUNNING);
		}
		else
		{
			scheduler->curr_thread = NULL;
		}
	}

	if (scheduler->curr_thread == NULL)
	{
		scheduler->curr_thread = scheduler->idle_thread;
	}

	// the thread changed so save the state of the previous thread
	if (current_thread != scheduler->curr_thread)
	{
		if (thread_state == THREAD_RUNNING)
		{
			thread_set_state(current_thread, THREAD_READY);
		}
		if (thread_state != THREAD_DEAD)
		{
			thread_save_context(current_thread, context);
		}
		// load in the state of the new thread
        tgt_prepare_context(context, scheduler->curr_thread, thread_get_parent(scheduler->curr_thread));
	}
	kernel_assert("Scheduler couldn't get next thread", scheduler->curr_thread != NULL);
}
