/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TIMER_H_
#define TIMER_H_

#include "api/tinker_api_time.h"
#include "time.h"

typedef void (timer_callback)(const tgt_context_t * const context);

typedef void* timer_param_t;
#define NO_TIMER_PARAM (timer_param_t)0

typedef void (timer_setup)(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back);

typedef void (timer_cancel)(
		const timer_param_t const usr_data);

typedef error_t (timer_isr)(const tgt_context_t * const context, timer_param_t param);

typedef struct
{
	timer_setup *		timer_setup;
	timer_cancel *		timer_cancel;
	timer_isr * 		timer_isr;
	timer_param_t		usr_data;
	uint32_t 			usr_data_size;
} timer_t;

#endif /* TIMER_H_ */
