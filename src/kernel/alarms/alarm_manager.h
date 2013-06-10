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

#ifndef ALARM_MANAGER_H_
#define ALARM_MANAGER_H_

#include "../kernel_types.h"

void __alarm_initialse(void);

void __alarm_set_timer(__timer_t * timer);

/**
 * Setup an alarm for a thread to a given call back
 * @param timeout The timeout from now
 * @param call_back The call back routine to execute
 * @param usr_data Call back data
 * @param usr_data_size The size of the call back data
 * @param alarm_id The ID of the alarm incase we have to cancel it
 * @return Errors
 */
error_t __alarm_set_alarm(
		__thread_t * thread,
		__time_t * timeout,
		__alarm_call_back * call_back,
		void * usr_data,
		uint32_t usr_data_size,
		uint32_t * alarm_id);

/**
 * Cancel a running alarm
 * @param alarm_id The alarm ID to cancel
 * @return Errors
 */
error_t __alarm_unset_alarm(uint32_t alarm_id);

#endif /* ALARM_MANAGER_H_ */
