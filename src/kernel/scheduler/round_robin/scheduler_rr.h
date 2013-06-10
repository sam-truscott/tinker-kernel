/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SCHEDULER_RR_H_
#define SCHEDULER_RR_H_

#include "kernel/scheduler/scheduler.h"

void __sc_rr_initialise(void);

void __sch_rr_execute(__thread_t ** new_thread);

__thread_t * __sch_rr_get_curr_thread(void);

void __sch_rr_set_curr_thread(__thread_t * thread);

#endif /* SCHEDULER_RR_H_ */
