/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TIMER_H_
#define TIMER_H_

#include "api/sos_api_time.h"
#include "time.h"

typedef void (__timer_callback)(void);

typedef void* __timer_param_t;
#define NO_TIMER_PARAM (__timer_param_t)0

typedef void (__timer_setup)(
		const __timer_param_t const usr_data,
		const sos_time_t timeout,
		__timer_callback * const call_back);

typedef void (__timer_cancel)(
		const __timer_param_t const usr_data);

typedef struct
{
	__timer_setup *		timer_setup;
	__timer_cancel *	timer_cancel;
	__timer_param_t		usr_data;
	uint32_t 			usr_data_size;
} __timer_t;

#endif /* TIMER_H_ */
