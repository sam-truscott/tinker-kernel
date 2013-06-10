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

#ifndef SCHEDULER_IMPL_H_
#define SCHEDULER_IMPL_H_

#include "../kernel_types.h"

void __sch_initialise(__scheduler_t * impl);

void __sch_initialise_scheduler(void);

void __sch_scheduler(void);

void __sch_notify_new_thread(__thread_t * t);
void __sch_notify_exit_thread(__thread_t * t);
void __sch_notify_pause_thread(__thread_t * t);
void __sch_notify_resume_thread(__thread_t * t);
void __sch_notify_change_priority(__thread_t * t, priority_t original_priority);

__thread_t * __sch_get_current_thread(void);

void __sch_set_current_thread(__thread_t * thread);

void __sch_prepare_scheduler_context(
		void * context,
		uint32_t context_size);

void __sch_save_context(
		void * context,
		uint32_t context_size);

void __sch_terminate_current_thread(void * context, uint32_t context_size);

#endif /* SCHEDULER_IMPL_H_ */
