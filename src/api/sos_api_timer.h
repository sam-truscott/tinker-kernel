/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_TIMER_H_
#define TINKER_API_TIMER_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"

typedef object_number_t sos_timer_t;

typedef struct
{
	uint32_t seconds;
	uint32_t nanoseconds;
} sos_timeout_time_t;

typedef void(sos_timer_callback_t)(const void * const usr_data);

error_t sos_timer_create(
		sos_timer_t * const timer,
		const uint8_t priority,
		const sos_timeout_time_t * const timeout,
		sos_timer_callback_t * const callback,
		const void * const data) TINKER_API_SUFFIX;

error_t sos_timer_cancel(sos_timer_t timer) TINKER_API_SUFFIX;

error_t sos_timer_delete(sos_timer_t timer) TINKER_API_SUFFIX;

#endif /* TINKER_API_TIMER_H_ */
