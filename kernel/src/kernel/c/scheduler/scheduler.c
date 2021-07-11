/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "scheduler/scheduler.h"
#include "scheduler/scheduler_private.h"

#include "tgt.h"
#include "board_support.h"

#include "kernel_assert.h"
#include "kernel_panic.h"
#include "process/process_list.h"
#include "objects/object_table.h"
#include "objects/obj_semaphore.h"
#include "utils/util_memcpy.h"

static void sch_priority_find_next_queue(scheduler_t * const scheduler, thread_t * const t);

scheduler_t * sch_create_scheduler(mem_pool_info_t * const pool)
{
	scheduler_t * const sch = mem_alloc(pool, sizeof(scheduler_t));
	if (sch)
	{
		util_memset(sch, 0, sizeof(scheduler_t));
		for (uint16_t i = 0 ; i < MAX_PRIORITY + 1 ; i++)
		{
			queue_initialise(&sch->priority_queues[i], mem_get_default_pool());
		}

		sch->curr_queue = &(sch->priority_queues[0]);
		stack_initialise( &sch->queue_stack, mem_get_default_pool() );
		stack_push(&sch->queue_stack, sch->curr_queue);
		sch->eval_new_thread = false;
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
	list_t * const list = stack_list(&scheduler->queue_stack);
	list_it_t stack_it;
	list_it_initialise(&stack_it, list);

	/*
	 * the stack queue uses the end of the queue (size) as the highest priority
	 * and the start (0) of the queue as the lowest priority
	 *
	 * --> the below code works out where to install the priority queue into
	 * --> the stack which is priority based
	 *
	 * FIXME: This is a poor linear search
	 * TODO: Binary search?
	 *
	 * Fix and use a binary search using double linked list via iterator fwd/back functions
	 *
	 */
	list_it_get(&stack_it, &stack_queue);
	while (stack_queue)
	{
		const uint32_t queue_size = queue_size(stack_queue);
		if (queue_size)
		{
			thread_t * first_thread_in_queue = NULL;
			if (queue_front(stack_queue, &first_thread_in_queue))
			{
				const priority_t stack_thread_priority = thread_get_priority(first_thread_in_queue);
				if (stack_thread_priority == thread_priority)
				{
					/* it's already on the stack */
					break;
				}
				else if (stack_thread_priority < thread_priority)
				{
					/*
					 * we need to insert the queue into the stack to ensure
					 * that at some point it'll be scheduled
					 */
					// TODO insert via the iterator? - avoids a second loop
					if (queue_insert(
							&scheduler->queue_stack,
							list_it_where(&stack_it),
							queue))
					{
						break;
					}
					else
					{
						/* failed to add it to the scheduler, panic */
						kernel_panic();
					}
				}
			}
		}
		list_it_next(&stack_it, &stack_queue);
	}
}

void sch_notify_new_thread(scheduler_t * const scheduler, thread_t * const t)
{
	if (scheduler && t)
	{
		const priority_t thread_priority = thread_get_priority(t);
		thread_queue_t * const queue = &(scheduler->priority_queues[thread_priority]);
		if (!queue_contains(queue, t))
		{
			debug_print(SCHEDULING_TRACE, "Push thread %s onto priority queue %d\n", thread_get_name(t), thread_priority);
			queue_push(queue, t);
		}

		debug_print(SCHEDULING, "Scheduler: New thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);

		/*
		 * a new thread that's higher than the current priority
		 */
		if (thread_priority > scheduler->curr_priority)
		{
			scheduler->eval_new_thread = true;
			scheduler->curr_priority = thread_priority;
			scheduler->curr_queue = &(scheduler->priority_queues[scheduler->curr_priority]);
			stack_push(&scheduler->queue_stack, scheduler->curr_queue);
		}
		/*
		 * a thread of lower or equal priority - inside it into the stack
		 */
		else if (thread_priority < scheduler->curr_priority)
		{
			insert_lower_priority_queue_to_stack(scheduler, thread_priority, queue);
		}
	}
	debug_print(SCHEDULING, "Scheduler: New priority, now (%d)\n", scheduler->curr_priority);
}

void sch_notify_exit_thread(scheduler_t * const scheduler, thread_t * const t)
{
	if (t)
	{
		const priority_t thread_priority = thread_get_priority(t);
		thread_queue_t * const queue = &(scheduler->priority_queues[thread_priority]);
		const bool_t removed = queue_remove(queue, t);

		debug_print(SCHEDULING, "Scheduler: Exit thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
		if (removed)
		{
			scheduler->eval_new_thread = true;
			// TODO add an isEmpty function to list, queue and empty
			if (queue_size(scheduler->curr_queue) == 0)
			{
				/* pop the one we're using off */
				stack_pop(&scheduler->queue_stack, &scheduler->curr_queue);
				/* pop the next queue off the stack - if one can't be found
				 * perform a slow search */
				if (stack_front(&scheduler->queue_stack, &scheduler->curr_queue))
				{
					thread_t * first_thread = NULL;
					if (queue_front(scheduler->curr_queue, &first_thread))
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
	}
	debug_print(SCHEDULING, "Scheduler: Exit priority, now (%d)\n", scheduler->curr_priority);
}

void sch_notify_terminated(scheduler_t * const scheduler, thread_t * const t)
{
	if (scheduler->curr_thread == t)
	{
		scheduler->curr_thread = NULL;
	}
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

		debug_print(SCHEDULING, "Scheduler: Change thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);

		/* remove it from the old list */
		queue_t * queue = &(scheduler->priority_queues[original_priority]);
		queue_remove(queue, t);

		/* add it to the new list */
		queue = &(scheduler->priority_queues[thread_priority]);
		queue_push(queue, t);

		// handle an empty priority queue - if it's empty remove it
		const bool_t is_curr_queue_empty = !queue_size(scheduler->curr_queue);
		if (is_curr_queue_empty)
		{
			/* pop the one we're using off */
			stack_pop(&scheduler->queue_stack, &scheduler->curr_queue);
		}
		if (thread_priority > original_priority)
		{
			scheduler->eval_new_thread = true;
			scheduler->curr_priority = thread_priority;
			scheduler->curr_queue = queue;
			stack_push(&scheduler->queue_stack, queue);
		}
		else
		{
			insert_lower_priority_queue_to_stack(scheduler, thread_priority, queue);
		}
		if (is_curr_queue_empty)
		{
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if (stack_front(&scheduler->queue_stack, &scheduler->curr_queue))
			{
				thread_t * new_thread = NULL;
				if (queue_front(scheduler->curr_queue, &new_thread))
				{
					scheduler->eval_new_thread = true;
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
	debug_print(SCHEDULING, "Scheduler: Change priority, now (%d)\n", scheduler->curr_priority);
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
		size = queue_size(queue);
		if ( size )
		{
			break;
		}
	} while (!size && p);

	scheduler->curr_priority = p;
	scheduler->curr_queue = queue;
	stack_push(&scheduler->queue_stack, scheduler->curr_queue);
}

void sch_set_context_for_next_thread(
		scheduler_t * const scheduler,
		tgt_context_t * const context,
		const thread_state_t thread_state)
{
	thread_t * const current_thread = scheduler->curr_thread;
	if (scheduler->curr_queue)
	{
		// FIXME do we need to swap thread if we're switching due an external
		// interrupt? Surely then we continue with the previous thread
		//
		// copy the head of the current queue as the current thread
		queue_front(scheduler->curr_queue, &scheduler->curr_thread);
	}

	/* once we're either back at the start or we've
	 * selected a new thread check its running and
	 * then use it */
	if (scheduler->curr_thread)
	{
		// move the head thread to the tail (round-robin)
		const bool_t reorder_ok = queue_reorder_first(scheduler->curr_queue);
		kernel_assert("re-ordering of priority queue failed", reorder_ok);
		const thread_state_t state = thread_get_state(scheduler->curr_thread);

		if (state == THREAD_READY || state == THREAD_SYSTEM || state == THREAD_RUNNING)
		{
			// FIXME setting it to THREAD_RUNNING if it's already RUNNING?
			thread_set_state(scheduler->curr_thread, THREAD_RUNNING);
		}
		else
		{
			scheduler->curr_thread = NULL;
		}
	}

	// if there's no current, default to idle
	if (scheduler->curr_thread == NULL)
	{
		scheduler->curr_thread = scheduler->idle_thread;
	}

	// the thread changed so save the state of the previous thread
	if ((current_thread != scheduler->curr_thread) || scheduler->eval_new_thread)
	{
		scheduler->eval_new_thread = false;
		if (thread_state == THREAD_RUNNING)
		{
			thread_set_state(current_thread, THREAD_READY);
		}
		if (thread_state != THREAD_DEAD)
		{
			thread_save_context(current_thread, context);
		}
		// load in the state of the new thread
		tgt_prepare_context(
				context,
				scheduler->curr_thread,
				/* FIXME if thread has been deleted the process will be gone
				 * and potenatially the process is dead too if all threads have gone */
				thread_get_parent(current_thread));
	}
	if (is_debug_enabled(SCHEDULING))
	{
		debug_print(SCHEDULING, "Scheduler: Current Thread is %s, new Thread is %s\n",
				current_thread == NULL ? "NULL" : thread_get_name(current_thread),
				scheduler->curr_thread == NULL ? "NULL" : thread_get_name(scheduler->curr_thread));
		debug_print(SCHEDULING, "Scheduler: Switching to %s\n", scheduler->curr_thread == NULL ? "NULL" : thread_get_name(scheduler->curr_thread));
		debug_print(SCHEDULING_TRACE, "Scheduler: Current queue size is %d\n", queue_size(scheduler->curr_queue));
	}
	if (queue_size(scheduler->curr_queue) > 1)
	{
		bsp_enable_schedule_timer();
	}
	kernel_assert("Scheduler couldn't get next thread", scheduler->curr_thread != NULL);
}
