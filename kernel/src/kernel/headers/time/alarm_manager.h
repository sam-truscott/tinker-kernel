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

#include "time/time.h"
#include "process/thread.h"
#include "time_manager.h"

typedef struct alarm_manager_t alarm_manager_t;

typedef void (timer_callback)(tgt_context_t * const context, void * const param);

typedef void* timer_param_t;
#define NO_TIMER_PARAM (timer_param_t)0

typedef void (timer_setup)(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back,
		void * const parm);

typedef void (timer_cancel)(
		const timer_param_t const usr_data);

typedef return_t (timer_isr)(tgt_context_t * const context, timer_param_t param);

typedef struct
{
	timer_setup *		timer_setup;
	timer_cancel *		timer_cancel;
	timer_isr * 		timer_isr;
	timer_param_t		usr_data;
	uint32_t 			usr_data_size;
} timer_t;

typedef void* alarm_user_data_t;

typedef void(alarm_call_back)(
		alarm_manager_t * const am,
		const uint32_t alarm_id,
		alarm_user_data_t const usr_data);

typedef struct alarm_t alarm_t;

alarm_t * alarm_create(
		mem_pool_info_t * const pool,
		const uint32_t id,
		tinker_time_t * const alarm_time,
		alarm_call_back * const callback,
		const alarm_user_data_t user_data);

alarm_manager_t * alarm_initialse(mem_pool_info_t * const pool, time_manager_t * const tm);

void alarm_set_timer(alarm_manager_t * const am, timer_t * const timer);

/**
 * Setup an alarm for a thread to a given call back
 * @param timeout The timeout from now
 * @param call_back The call back routine to execute
 * @param usr_data Call back data
 * @param usr_data_size The size of the call back data
 * @param alarm_id The ID of the alarm incase we have to cancel it
 * @return Errors
 */
return_t alarm_set_alarm(
		alarm_manager_t * const am,
		const tinker_time_t * const timeout,
		alarm_call_back * const call_back,
		const alarm_user_data_t const usr_data,
		uint32_t * const alarm_id);

/**
 * Cancel a running alarm
 * @param alarm_id The alarm ID to cancel
 * @return Errors
 */
return_t alarm_unset_alarm(
		alarm_manager_t * const am,
		const uint32_t alarm_id);

void alarm_delete(alarm_t * const alarm);

const tinker_time_t* alarm_get_time(const alarm_t * const alarm);

#endif /* ALARM_MANAGER_H_ */
