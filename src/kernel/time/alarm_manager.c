/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "alarm_manager.h"

#include "config.h"
#include "kernel/process/process.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/time/time.h"
#include "kernel/time/time_manager.h"
#include "kernel/utils/collections/unbounded_list.h"

UNBOUNDED_LIST_TYPE(alarm_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_ADD(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_REMOVE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_REMOVE_ITEM(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_GET(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_SIZE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_CREATE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_ADD(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_REMOVE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_GET(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_SIZE(static, alarm_list_t, alarm_t*)

static void alarm_calculate_next_alarm(alarm_t * const new_alarm);

static void alarm_enable_timer(void);

static void alarm_disable_timer(void);

static void alarm_handle_timer_timeout(void);

static alarm_list_t * alarm_list;

static alarm_t * alarm_next_alarm;

static timer_t * alarm_timer;

void alarm_initialse(mem_pool_info_t * const pool)
{
	alarm_timer = NULL;
	alarm_next_alarm = NULL;
	alarm_list = alarm_list_t_create(pool);
}

void alarm_set_timer(timer_t * const timer)
{
	alarm_timer = timer;
}

error_t alarm_set_alarm(
		mem_pool_info_t * const pool,
		const tinker_time_t * const timeout,
		alarm_call_back * const call_back,
		const alarm_user_data_t const usr_data,
		uint32_t * const alarm_id)
{
	error_t ret = NO_ERROR;
	if (pool && timeout)
	{
	    tinker_time_t now = TINKER_ZERO_TIME;
		time_get_system_time(&now);

		/* check there's room of the new alarm */
		const uint32_t alarm_list_size = alarm_list_t_size(alarm_list);
		if (alarm_list_size < MAX_ALARMS)
		{
			alarm_t * tmp = NULL;
			/* find the next valid alarm ID TODO this should be an operation
			 * of the unbound_list as the process manager also needs to do it
			 */
			uint32_t new_alarm_id = 0;
			for (uint32_t i = 0 ; i < alarm_list_size ; i++)
			{
				if ( !alarm_list_t_get(alarm_list, i, &tmp) )
				{
					new_alarm_id = i;
					break;
				}
			}

			tinker_time_t alarm_time;
			tinker_time_add(&now, timeout, &alarm_time);
			alarm_t * const new_alarm = alarm_create(
					pool,
					new_alarm_id,
					&alarm_time,
					call_back,
					usr_data);
			if (new_alarm)
			{
				if (alarm_list_t_add(alarm_list, new_alarm))
				{
					alarm_calculate_next_alarm(new_alarm);
					if (alarm_id)
					{
						*alarm_id = new_alarm_id;
					}
					if (alarm_next_alarm)
					{
						alarm_enable_timer();
					}
				}
				else
				{
					ret = OUT_OF_MEMORY;
				}
			}
			else
			{
				ret = OUT_OF_MEMORY;
			}
		}
		else
		{
			ret = OUT_OF_MEMORY;
		}
	}
	else
	{
		ret = PARAMETERS_INVALID;
	}
	return ret;
}

/**
 * Cancel a running alarm
 * @param alarm_id The alarm ID to cancel
 * @return Errors
 */
error_t alarm_unset_alarm(const uint32_t alarm_id)
{
	error_t ret = NO_ERROR;

	alarm_t * alarm = NULL;
	const bool_t got = alarm_list_t_get(alarm_list, alarm_id, &alarm);
	if (got && alarm)
	{
		alarm_list_t_remove_item(alarm_list, alarm);
		if (alarm == alarm_next_alarm)
		{
			alarm_disable_timer();
			alarm_next_alarm = NULL;
			alarm_calculate_next_alarm(NULL);
			alarm_enable_timer();
		}
		alarm_delete(alarm);
	}
	else
	{
		ret = ALARM_ID_UNKNOWN;
	}

	return ret;
}

void alarm_calculate_next_alarm(alarm_t * const new_alarm)
{
	if (new_alarm)
	{
		if (alarm_next_alarm == NULL)
		{
			alarm_next_alarm = new_alarm;
			alarm_enable_timer();
		}
		else if (tinker_time_lt(
				alarm_get_time(new_alarm),
				alarm_get_time(alarm_next_alarm)))
		{
			alarm_next_alarm = new_alarm;
			alarm_enable_timer();
		}
	}
	else
	{
		if (alarm_list_t_size(alarm_list) > 0)
		{
			alarm_list_t_get(alarm_list, 0, &alarm_next_alarm);
			alarm_enable_timer();
		}
	}
}

void alarm_handle_timer_timeout(void)
{
	if (alarm_next_alarm)
	{
		alarm_fire_callback(alarm_next_alarm);
		alarm_calculate_next_alarm(NULL);
	}
}

void alarm_enable_timer(void)
{
	if (alarm_timer && alarm_next_alarm)
	{
		alarm_timer->timer_setup(
				alarm_timer->usr_data,
				alarm_get_time(alarm_next_alarm),
				alarm_handle_timer_timeout);
	}
}

void alarm_disable_timer(void)
{
	if (alarm_timer)
	{
		alarm_timer->timer_cancel(alarm_timer->usr_data);
	}
}
