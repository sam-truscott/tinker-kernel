/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "sos_api_timer.h"
#include "sos_api_kernel_interface.h"

error_t sos_timer_create(
		sos_timer_t * const timer,
		const sos_timeout_time_t * const timeout,
		sos_timer_callback_t * const callback,
		const void * const data)
{
	return SOS_API_CALL_5(
			SYSCALL_CREATE_TIMER,
			(uint32_t)timer,
			(uint32_t)timeout->seconds,
			(uint32_t)timeout->nanoseconds,
			(uint32_t)callback,
			(uint32_t)data);
}

error_t sos_timer_cancel(sos_timer_t * const timer)
{
	return SOS_API_CALL_1(
			SYSCALL_CANCEL_TIMER,
			(uint32_t)timer);
}

error_t sos_timer_delete(sos_timer_t * const timer)
{
	return SOS_API_CALL_1(
			SYSCALL_DELETE_TIMER,
			(uint32_t)timer);
}
