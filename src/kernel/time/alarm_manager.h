/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef ALARM_MANAGER_H_
#define ALARM_MANAGER_H_

#include "kernel/time/time.h"
#include "kernel/time/timer.h"
#include "kernel/time/alarm.h"
#include "kernel/process/thread.h"

void alarm_initialse(mem_pool_info_t * const pool);

void alarm_set_timer(timer_t * const timer);

/**
 * Setup an alarm for a thread to a given call back
 * @param timeout The timeout from now
 * @param call_back The call back routine to execute
 * @param usr_data Call back data
 * @param usr_data_size The size of the call back data
 * @param alarm_id The ID of the alarm incase we have to cancel it
 * @return Errors
 */
error_t alarm_set_alarm(
		mem_pool_info_t * const pool,
		const tinker_time_t * const timeout,
		alarm_call_back * const call_back,
		const alarm_user_data_t const usr_data,
		uint32_t * const alarm_id);

/**
 * Cancel a running alarm
 * @param alarm_id The alarm ID to cancel
 * @return Errors
 */
error_t alarm_unset_alarm(const uint32_t alarm_id);

#endif /* ALARM_MANAGER_H_ */
