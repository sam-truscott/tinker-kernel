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

#include "kernel/process/thread.h"

void sch_initialise_scheduler(void);

void sch_notify_new_thread(thread_t * const t);
void sch_notify_exit_thread(thread_t * const t);
void sch_notify_pause_thread(thread_t * const t);
void sch_notify_resume_thread(thread_t * const  t);
void sch_notify_change_priority(
		thread_t * const t,
		const  priority_t original_priority);

thread_t * sch_get_current_thread(void);

void sch_set_current_thread(thread_t * const thread);

void sch_terminate_current_thread(
		const tgt_context_t * const context);

void sch_set_context_for_next_thread(
		tgt_context_t * const context,
		const thread_state_t thread_state);

#endif /* SCHEDULER_IMPL_H_ */
