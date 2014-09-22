/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "ppc32tbr_timer.h"

#include "kernel/memory/memory_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/utils/util_memcpy.h"

typedef struct
{
	bool_t enabled;
	tinker_time_t alarm_time;
	timer_callback * call_back;
} ppc_timer_usr_data_t;

static void ppc_timer_setup(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back);

static void ppc_timer_cancel(const timer_param_t const usr_data);

void ppc_get_timer(const process_t * const parent, timer_t * const timer)
{
	if (parent && timer)
	{
		timer->timer_setup = ppc_timer_setup;
		timer->timer_cancel = ppc_timer_cancel;
		timer->usr_data = (timer_param_t)mem_alloc(process_get_mem_pool(parent), sizeof(ppc_timer_usr_data_t));
		if (timer->usr_data)
		{
			timer->usr_data_size = sizeof(ppc_timer_usr_data_t);
		}
		else
		{
			timer->usr_data = NO_TIMER_PARAM;
			timer->usr_data_size = 0;
		}
	}
}

void ppc_check_timer(timer_t * const timer, const tgt_context_t * const context)
{
	if (timer)
	{
		ppc_timer_usr_data_t * const data = (ppc_timer_usr_data_t*)timer->usr_data;
		if (data->enabled)
		{
		    tinker_time_t now = TINKER_ZERO_TIME;
			time_get_system_time(&now);
			if (tinker_time_gt(&now, &data->alarm_time))
			{
				data->enabled = false;
				data->call_back(context);
			}
		}
	}
}

void ppc_timer_setup(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back)
{
	if (usr_data && call_back)
	{
		ppc_timer_usr_data_t * const data = (ppc_timer_usr_data_t*)usr_data;
		data->call_back = call_back;
		data->alarm_time = *timeout;
		data->enabled = true;
	}
}

void ppc_timer_cancel(const timer_param_t const usr_data)
{
	if (usr_data)
	{
		ppc_timer_usr_data_t * const data = (ppc_timer_usr_data_t*)usr_data;
		if (data)
		{
			*((uint32_t*)usr_data) = 0;
		}
	}
}
