/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "tinker_api_timer.h"
#include "tinker_api_kernel_interface.h"

return_t tinker_timer_create(
		tinker_timer_t * const timer,
		const uint8_t priority,
		const tinker_timeout_time_t * const timeout,
		tinker_timer_callback_t * const callback,
		const void * const data)
{
	return TINKER_API_CALL_6(
			SYSCALL_CREATE_TIMER,
			(uint32_t)timer,
			(uint32_t)priority,
			(uint32_t)timeout->seconds,
			(uint32_t)timeout->nanoseconds,
			(uint32_t)callback,
			(uint32_t)data);
}

return_t tinker_timer_cancel(tinker_timer_t timer)
{
	return TINKER_API_CALL_1(
			SYSCALL_CANCEL_TIMER,
			(uint32_t)timer);
}

return_t tinker_timer_delete(tinker_timer_t timer)
{
	return TINKER_API_CALL_1(
			SYSCALL_DELETE_TIMER,
			(uint32_t)timer);
}
