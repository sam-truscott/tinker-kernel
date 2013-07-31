/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SCHEDULER_IMPL_H_
#define SCHEDULER_IMPL_H_

#include "kernel/process/thread.h"

void __sch_initialise_scheduler(void);

void __sch_notify_new_thread(__thread_t * const t);
void __sch_notify_exit_thread(__thread_t * const t);
void __sch_notify_pause_thread(__thread_t * const t);
void __sch_notify_resume_thread(__thread_t * const  t);
void __sch_notify_change_priority(
		__thread_t * const t,
		const  priority_t original_priority);

__thread_t * __sch_get_current_thread(void);

void __sch_set_current_thread(__thread_t * const thread);

void __sch_terminate_current_thread(
		const __tgt_context_t * const context);

void __sch_set_context_for_next_thread(
		__tgt_context_t * const context);

#endif /* SCHEDULER_IMPL_H_ */
