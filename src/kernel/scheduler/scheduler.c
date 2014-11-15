/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "scheduler.h"

#include "arch/tgt.h"
#include "arch/board_support.h"

#include "kernel/kernel_assert.h"
#include "kernel/kernel_panic.h"
#include "kernel/kernel_initialise.h"
#include "kernel/process/process_manager.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/utils/collections/unbounded_queue.h"
#include "kernel/utils/collections/stack.h"

UNBOUNDED_QUEUE_TYPE(thread_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_INITIALISE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_FRONT(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_PUSH(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_POP(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_REMOVE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_REORDER_FIRST(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_SIZE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_INITIALISE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_FRONT(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_PUSH(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_POP(extern, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_REMOVE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_REORDER_FIRST(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_SIZE(static, thread_queue_t, thread_t*)

STACK_TYPE(queue_stack_t)
STACK_INTERNAL_TYPE(queue_stack_t, thread_queue_t*)
STACK_SPEC_INITIALISE(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_PUSH(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_POP(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_SIZE(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_GET(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_FRONT(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_INSERT(static, queue_stack_t, thread_queue_t*)
STACK_BODY_INITIALISE(static, queue_stack_t, thread_queue_t*)
STACK_BODY_PUSH(static, queue_stack_t, thread_queue_t*)
STACK_BODY_SIZE(static, queue_stack_t, thread_queue_t*)
STACK_BODY_GET(static, queue_stack_t, thread_queue_t*)
STACK_BODY_POP(static, queue_stack_t, thread_queue_t*)
STACK_BODY_FRONT(static, queue_stack_t, thread_queue_t*)
STACK_BODY_INSERT(static, queue_stack_t, thread_queue_t*)

/**
 * The current thread executing
 */
static thread_t * sch_current_thread = NULL;

static process_t * sch_current_process = NULL;

static thread_queue_t * sch_active_queue;

static thread_queue_t sch_thread_queues[MAX_PRIORITY + 1];

static priority_t sch_current_priority;

static queue_stack_t sch_queue_stack;

static void sch_priority_find_next_queue(thread_t * const t);

static bool_t sch_initialised = false;

void sch_initialise_scheduler(void)
{
	if ( !sch_initialised )
	{
		for ( uint16_t i = 0 ; i < MAX_PRIORITY + 1 ; i++ )
		{
			thread_queue_t_initialise(&sch_thread_queues[i], mem_get_default_pool());
		}

		sch_active_queue = &(sch_thread_queues[0]);
		queue_stack_t_initialise( &sch_queue_stack, mem_get_default_pool() );
		queue_stack_t_push(&sch_queue_stack, sch_active_queue);

		sch_initialised = true;
	}
}

void sch_notify_new_thread(thread_t * const t)
{
	if (!sch_initialised)
	{
		sch_initialise_scheduler();
	}

	if (t)
	{
		const priority_t thread_priority = thread_get_priority(t);
		thread_queue_t * const queue = &(sch_thread_queues[thread_priority]);
		thread_queue_t_push(queue, t);

#if defined(PROCESS_DEBUGGING)
		debug_print("Scheduler: New thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
#endif

		if (thread_priority > sch_current_priority)
		{
				sch_current_priority = thread_priority;
				sch_active_queue = &(sch_thread_queues[sch_current_priority]);
				queue_stack_t_push(&sch_queue_stack, sch_active_queue);
		}
		else if ( thread_priority < sch_current_priority )
		{
			/* if we add a thread that's of a lower priority then it must be added
			 * to the queue stack so when the new high priority task finishes the
			 *  new lower priority task will be executed */
			thread_queue_t * stack_queue = NULL;
			const uint32_t stack_size = queue_stack_t_size(&sch_queue_stack);
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
			while(stack_index >= 0 && !found && insert_index == -1)
			{
				if (queue_stack_t_get(&sch_queue_stack, stack_index, &stack_queue))
				{
					const uint32_t queue_size = thread_queue_t_size(stack_queue);
					if ( queue_size )
					{
						thread_t * firstThreadInQueue = NULL;
						const bool_t ok = thread_queue_t_front(stack_queue, &firstThreadInQueue);
						if ( ok == true )
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
				if (!queue_stack_t_insert(&sch_queue_stack, insert_index, queue))
				{
					/* failed to add it to the scheduler, panic */
					kernel_panic();
				}
			}
		}
	}
#if defined(PROCESS_DEBUGGING)
		debug_print("Scheduler: New priority, now (%d)\n", sch_current_priority);
#endif
}

void sch_notify_exit_thread(thread_t * const t)
{
	if (t)
	{
		const priority_t thread_priority = thread_get_priority(t);
		thread_queue_t * const queue = &(sch_thread_queues[thread_priority]);
		const bool_t removed = thread_queue_t_remove(queue, t);

#if defined(PROCESS_DEBUGGING)
		debug_print("Scheduler: Exit thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
#endif

		if (removed && thread_queue_t_size(sch_active_queue) == 0)
		{
			/* pop the one we're using off */
			queue_stack_t_pop(&sch_queue_stack, &sch_active_queue);
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if (!queue_stack_t_front(&sch_queue_stack, &sch_active_queue))
			{
				sch_priority_find_next_queue(t);
			}
			else
			{
				thread_t * first_thread = NULL;
				if (thread_queue_t_front(sch_active_queue, &first_thread))
				{
					const priority_t tp = thread_get_priority(first_thread);
					sch_current_priority = tp;
				}
			}
		}
	}
#if defined(PROCESS_DEBUGGING)
		debug_print("Scheduler: Exit priority, now (%d)\n", sch_current_priority);
#endif
}

void sch_notify_pause_thread(thread_t * const t)
{
	sch_notify_exit_thread(t);
}

void sch_notify_resume_thread(thread_t * const t)
{
	sch_notify_new_thread(t);
}

void sch_notify_change_priority(
		thread_t * const t,
		const priority_t original_priority)
{
	if ( t )
	{
		const priority_t thread_priority = thread_get_priority(t);

#if defined(PROCESS_DEBUGGING)
		debug_print("Scheduler: Change thread (%s) with priority (%d)\n", thread_get_name(t), thread_priority);
#endif

		/* remove it from the old list */
		thread_queue_t * queue = &(sch_thread_queues[original_priority]);
		thread_queue_t_remove(queue, t);

		/* add it to the new list */
		queue = &(sch_thread_queues[thread_priority]);
		thread_queue_t_push(queue, t);

		if (!thread_queue_t_size(sch_active_queue))
		{
			sch_current_priority = thread_priority;

			/* pop the one we're using off */
			queue_stack_t_pop(&sch_queue_stack, &sch_active_queue);
			/* pop the next queue off the stack - if one can't be found
			 * perform a slow search */
			if ( !queue_stack_t_front(&sch_queue_stack, &sch_active_queue) )
			{
				sch_priority_find_next_queue(t);
			}
		}
	}
#if defined(PROCESS_DEBUGGING)
		debug_print("Scheduler: Change priority, now (%d)\n", sch_current_priority);
#endif
}

void sch_terminate_current_thread(
		const tgt_context_t * const context)
{
	thread_t * const thread = sch_get_current_thread();
	kernel_assert("Attempted to terminate idle thread\n", thread!=kernel_get_idle_thread());
	if (thread)
	{
		thread_set_state(thread, THREAD_TERMINATED);
		thread_save_context(thread, context);
		sch_notify_exit_thread(thread);
	}
}

thread_t * sch_get_current_thread(void)
{
	return sch_current_thread;
}

void sch_set_current_thread(thread_t * const thread)
{
	if (thread)
	{
		sch_current_thread = thread;
		sch_current_process = thread_get_parent(thread);
	}
}

static void sch_priority_find_next_queue(thread_t * const t)
{
	uint32_t size;
	priority_t p = thread_get_priority(t);
	thread_queue_t * queue;

	/* there will always be 1 - the idle thread */
	do
	{
		queue = &(sch_thread_queues[--p]);
		size = thread_queue_t_size(queue);
		if ( size )
		{
			break;
		}
	} while (!size && p);

	sch_current_priority = p;
	sch_active_queue = queue;
	queue_stack_t_push(&sch_queue_stack, sch_active_queue);
}

void sch_set_context_for_next_thread(
		tgt_context_t * const context,
		const thread_state_t thread_state)
{
	thread_t * const current_thread = sch_current_thread;

	if (sch_active_queue)
	{
		thread_queue_t_front(sch_active_queue, &sch_current_thread);
	}

	/* once we're either back at the start or we've
	 * selected a new thread check its running and
	 * then use it */
	if (sch_current_thread)
	{
		const bool_t reorder_ok = thread_queue_t_reorder_first(sch_active_queue);
		kernel_assert("re-ordering of priority queue failed", reorder_ok);
		const thread_state_t state = thread_get_state(sch_current_thread);
		if (state == THREADY_READY)
		{
			thread_set_state(sch_current_thread, THREAD_RUNNING);
		}
		else
		{
			sch_current_thread = NULL;
		}
	}

	if (sch_current_thread == NULL)
	{
		sch_current_thread = kernel_get_idle_thread();
	}

	// the thread changed so save the state of the previous thread
	if (current_thread != sch_current_thread)
	{
		if (thread_state == THREAD_RUNNING)
		{
			thread_set_state(current_thread, THREADY_READY);
		}
		thread_save_context(current_thread, context);
		// load in the state of the new thread
        tgt_prepare_context(context, sch_current_thread, sch_current_process);
        // update the current process after the switch
        sch_current_process = thread_get_parent(sch_current_thread);
	}

	kernel_assert("Scheduler couldn't get next thread", sch_current_thread != NULL);
}
