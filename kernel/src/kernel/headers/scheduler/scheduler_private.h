/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_SCHEDULER_SCHEDULER_PRIVATE_H_
#define KERNEL_SCHEDULER_SCHEDULER_PRIVATE_H_

#include "config.h"
#include "utils/collections/unbounded_queue.h"
#include "utils/collections/stack.h"
#include "process/thread.h"

UNBOUNDED_QUEUE_TYPE(thread_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_INITIALISE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_FRONT(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_PUSH(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_POP(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_REMOVE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_REORDER_FIRST(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_SPEC_SIZE(static, thread_queue_t)
UNBOUNDED_QUEUE_SPEC_CONTAINS(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_INITIALISE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_FRONT(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_PUSH(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_POP(extern, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_REMOVE(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_REORDER_FIRST(static, thread_queue_t, thread_t*)
UNBOUNDED_QUEUE_BODY_SIZE(static, thread_queue_t)
UNBOUNDED_QUEUE_BODY_CONTAINS(static, thread_queue_t, thread_t*)

STACK_TYPE(queue_stack_t)
STACK_INTERNAL_TYPE(queue_stack_t, thread_queue_t*)
STACK_SPEC_INITIALISE(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_PUSH(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_POP(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_SIZE(static, queue_stack_t)
STACK_SPEC_GET(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_FRONT(static, queue_stack_t, thread_queue_t*)
STACK_SPEC_INSERT(static, queue_stack_t, thread_queue_t*)
STACK_BODY_INITIALISE(static, queue_stack_t, thread_queue_t*)
STACK_BODY_PUSH(static, queue_stack_t, thread_queue_t*)
STACK_BODY_SIZE(static, queue_stack_t)
STACK_BODY_GET(static, queue_stack_t, thread_queue_t*)
STACK_BODY_POP(static, queue_stack_t, thread_queue_t*)
STACK_BODY_FRONT(static, queue_stack_t, thread_queue_t*)
STACK_BODY_INSERT(static, queue_stack_t, thread_queue_t*)

typedef struct scheduler_t
{
	/**
	 * The current thread being executed by the scheduler
	 */
	thread_t * curr_thread;
	/**
	 * The current queue of threads being processed (round robin)
	 */
	thread_queue_t * curr_queue;
	/**
	 * All the priority round-robin queues
	 */
	thread_queue_t priority_queues[MAX_PRIORITY + 1];
	/**
	 * The current scheduler priority
	 */
	priority_t curr_priority;
	/**
	 * The scheduler stack of queued round robins
	 */
	queue_stack_t queue_stack;
	/**
	 * The scheduler's idle thread
	 */
	thread_t * idle_thread;
	/**
	 * Has a scheduler event changed the current thread
	 */
	bool_t eval_new_thread;
} scheduler_t;



#endif /* KERNEL_SCHEDULER_SCHEDULER_PRIVATE_H_ */
