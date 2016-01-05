/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SCHEDULER_IMPL_H_
#define SCHEDULER_IMPL_H_

#include "process/thread.h"

typedef struct scheduler_t scheduler_t;

scheduler_t * sch_create_scheduler(mem_pool_info_t * const pool);

void sch_set_kernel_idle_thread(scheduler_t * const sch, thread_t * const idle_thread);

void sch_notify_new_thread(scheduler_t * const scheduler, thread_t * const t);
void sch_notify_exit_thread(scheduler_t * const scheduler, thread_t * const t);
void sch_notify_terminated(scheduler_t * const scheduler, thread_t * const t);
void sch_notify_pause_thread(scheduler_t * const scheduler, thread_t * const t);
void sch_notify_resume_thread(scheduler_t * const scheduler, thread_t * const  t);
void sch_notify_change_priority(
		scheduler_t * const scheduler,
		thread_t * const t,
		const  priority_t original_priority);

thread_t * sch_get_current_thread(scheduler_t * const scheduler);

void sch_set_current_thread(scheduler_t * const scheduler, thread_t * const thread);

void sch_terminate_current_thread(
		scheduler_t * const scheduler,
		const tgt_context_t * const context);

void sch_set_context_for_next_thread(
		scheduler_t * const scheduler,
		tgt_context_t * const context,
		const thread_state_t thread_state);

#endif /* SCHEDULER_IMPL_H_ */
