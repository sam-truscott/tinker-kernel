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
#include "utils/collections/unbounded_list.h"
#include "process/thread.h"

typedef struct scheduler_t
{
	/**
	 * The current thread being executed by the scheduler
	 */
	thread_t * curr_thread;
	/**
	 * The current queue of threads being processed (round robin)
	 */
	queue_t * curr_queue;
	/**
	 * All the priority round-robin queues
	 */
	queue_t priority_queues[MAX_PRIORITY + 1];
	/**
	 * The current scheduler priority
	 */
	priority_t curr_priority;
	/**
	 * The scheduler stack of queued round robins
	 */
	stack_t queue_stack;
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
