/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef SCHEDULER_RR_H_
#define SCHEDULER_RR_H_

#include "kernel/scheduler/scheduler.h"

void __sc_rr_initialise(void);

void __sch_rr_execute(__thread_t ** new_thread);

__thread_t * __sch_rr_get_curr_thread(void);

void __sch_rr_set_curr_thread(__thread_t * thread);

#endif /* SCHEDULER_RR_H_ */
