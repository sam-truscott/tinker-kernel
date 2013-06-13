/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SCHEDULER_PRIORITY_H_
#define SCHEDULER_PRIORITY_H_

#include "kernel/scheduler/scheduler.h"

void __sch_priority_initialise(void);

void __sch_priority_execute(__thread_t ** new_thread);

void __sch_priority_notify_new_thread(__thread_t * const t);
void __sch_priority_notify_exit_thread(__thread_t * const t);
void __sch_priority_notify_pause_thread(__thread_t * const t);
void __sch_priority_notify_resume_thread(__thread_t * const t);
void __sch_priority_notify_change_priority(
		__thread_t * const t,
		const priority_t original_priority);

__thread_t * __sch_priority_get_curr_thread(void);

void __sch_priority_set_curr_thread(__thread_t * const thread);

#endif /* SCHEDULER_PRIORITY_H_ */
