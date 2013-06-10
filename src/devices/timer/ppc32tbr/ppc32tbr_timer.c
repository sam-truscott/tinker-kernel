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

#include "ppc32tbr_timer.h"

#include "kernel/memory/memory_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/time_utilities.h"

typedef struct
{
	bool enabled;
	__time_t alarm_time;
	__timer_callback * call_back;
} __ppc_timer_usr_data_t;

static void __ppc_timer_setup(void * usr_data,__time_t timeout, __timer_callback * call_back);

static void __ppc_timer_cancel(void * usr_data);

void __ppc_get_timer(__process_t * parent, __timer_t * timer)
{
	if ( parent && timer )
	{
		timer->timer_setup = __ppc_timer_setup;
		timer->timer_cancel = __ppc_timer_cancel;
		timer->usr_data = __mem_alloc(parent->memory_pool, sizeof(__ppc_timer_usr_data_t));
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

void __ppc_check_timer(__timer_t * timer)
{
	if ( timer )
	{
		__ppc_timer_usr_data_t * data = (__ppc_timer_usr_data_t*)timer->usr_data;
		if ( data->enabled )
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

void __ppc_timer_setup(void * usr_data,__time_t timeout, __timer_callback * call_back)
{
	if ( usr_data && call_back)
	{
		__ppc_timer_usr_data_t * data = (__ppc_timer_usr_data_t*)usr_data;
		data->call_back = call_back;
		data->alarm_time = timeout;
		data->enabled = true;
	}
}

void __ppc_timer_cancel(void * usr_data)
{
	if ( usr_data )
	{
		*((uint32_t*)usr_data) = 0;
	}
}
