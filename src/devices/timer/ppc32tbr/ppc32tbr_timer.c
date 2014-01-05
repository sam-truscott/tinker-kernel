/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "ppc32tbr_timer.h"

#include "kernel/memory/memory_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/time_utilities.h"

typedef struct
{
	bool_t enabled;
	__time_t alarm_time;
	__timer_callback * call_back;
} __ppc_timer_usr_data_t;

static void __ppc_timer_setup(
		const void * const usr_data,
		const __time_t timeout,
		__timer_callback * const call_back);

static void __ppc_timer_cancel(const void * const usr_data);

void __ppc_get_timer(const __process_t * const parent, __timer_t * const timer)
{
	if ( parent && timer )
	{
		timer->timer_setup = __ppc_timer_setup;
		timer->timer_cancel = __ppc_timer_cancel;
		timer->usr_data = __mem_alloc(__process_get_mem_pool(parent), sizeof(__ppc_timer_usr_data_t));
		if ( timer->usr_data )
		{
			timer->usr_data_size = sizeof(__ppc_timer_usr_data_t);


		}
		else
		{
			timer->usr_data = NULL;
			timer->usr_data_size = 0;
		}
	}
}

void __ppc_check_timer(__timer_t * const timer)
{
	if (timer)
	{
		__ppc_timer_usr_data_t * const data = (__ppc_timer_usr_data_t*)timer->usr_data;
		if (data->enabled)
		{
			const __time_t now = __time_get_system_time();
			if ( __time_gt(now, data->alarm_time))
			{
				data->enabled = false;
				data->call_back();
			}
		}
	}
}

void __ppc_timer_setup(
		const void * const usr_data,
		const __time_t timeout,
		__timer_callback * const call_back)
{
	if ( usr_data && call_back)
	{
		__ppc_timer_usr_data_t * data = (__ppc_timer_usr_data_t*)usr_data;
		data->call_back = call_back;
		data->alarm_time = timeout;
		data->enabled = true;
	}
}

void __ppc_timer_cancel(const void * const usr_data)
{
	if ( usr_data )
	{
		*((uint32_t*)usr_data) = 0;
	}
}
