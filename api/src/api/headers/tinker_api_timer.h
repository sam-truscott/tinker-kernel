/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_TIMER_H_
#define TINKER_API_TIMER_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"

typedef object_number_t tinker_timer_t;

typedef struct
{
	uint32_t seconds;
	uint32_t nanoseconds;
} tinker_timeout_time_t;

typedef void(tinker_timer_callback_t)(const void * const usr_data);

return_t tinker_timer_create(
		tinker_timer_t * const timer,
		const uint8_t priority,
		const tinker_timeout_time_t * const timeout,
		tinker_timer_callback_t * const callback,
		const void * const data) TINKER_API_SUFFIX;

return_t tinker_timer_cancel(tinker_timer_t timer) TINKER_API_SUFFIX;

return_t tinker_timer_delete(tinker_timer_t timer) TINKER_API_SUFFIX;

#endif /* TINKER_API_TIMER_H_ */
