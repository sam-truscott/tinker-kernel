/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "time/alarm_manager.h"

#include "config.h"
#include "process/process.h"
#include "memory/memory_manager.h"
#include "time/time.h"
#include "time/time_manager.h"
#include "utils/collections/unbounded_list.h"

UNBOUNDED_LIST_TYPE(alarm_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_ADD(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_REMOVE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_REMOVE_ITEM(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_GET(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_SPEC_SIZE(static, alarm_list_t)
UNBOUNDED_LIST_BODY_CREATE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_ADD(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_REMOVE(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_GET(static, alarm_list_t, alarm_t*)
UNBOUNDED_LIST_BODY_SIZE(static, alarm_list_t)

typedef struct alarm_manager_t
{
	alarm_list_t * alarm_list;
	alarm_t * next_alarm;
	timer_t * alarm_timer;
	mem_pool_info_t * pool;
	time_manager_t * time_manager;
} alarm_manager_t;

typedef struct alarm_t
{
	uint32_t 			id;
	alarm_call_back *	call_back;
	void * 				usr_data;
	tinker_time_t			alarm_time;
	mem_pool_info_t * pool;
} alarm_internal_t;

static void alarm_calculate_next_alarm(alarm_manager_t * const am, alarm_t * const new_alarm);

static void alarm_enable_timer(alarm_manager_t * const am);

static void alarm_disable_timer(alarm_manager_t * const am);

static void alarm_handle_timer_timeout(tgt_context_t * const context, void * const param);

static void alarm_fire_callback(alarm_manager_t * const am);

alarm_manager_t * alarm_initialse(mem_pool_info_t * const pool, time_manager_t * const tm)
{
	alarm_manager_t * const am = mem_alloc(pool, sizeof(alarm_manager_t));
	if (am)
	{
		am->alarm_timer = NULL;
		am->next_alarm = NULL;
		am->alarm_list = alarm_list_t_create(pool);
		am->pool = pool;
		am->time_manager = tm;
	}
	return am;
}

void alarm_set_timer(alarm_manager_t * const am, timer_t * const timer)
{
	if (am)
	{
		am->alarm_timer = timer;
	}
}

error_t alarm_set_alarm(
		alarm_manager_t * const am,
		const tinker_time_t * const timeout,
		alarm_call_back * const call_back,
		const alarm_user_data_t const usr_data,
		uint32_t * const alarm_id)
{
	error_t ret = NO_ERROR;
	if (is_debug_enabled(ALARM))
	{
		if (timeout)
		{
			debug_print(ALARM, "Alarms: Setting up a new alarm for %d.%d\n", timeout->seconds, timeout->nanoseconds);
		}
		else
		{
			debug_prints(ALARM, "Alarms: Setting up a new alarm for NULL\n");
		}
	}
	if (am && timeout)
	{
	    tinker_time_t now = TINKER_ZERO_TIME;
		time_get_system_time(am->time_manager, &now);

		/* check there's room of the new alarm */
		const uint32_t alarm_list_size = alarm_list_t_size(am->alarm_list);
		if (alarm_list_size < MAX_ALARMS)
		{
			alarm_t * tmp = NULL;
			/* find the next valid alarm ID TODO this should be an operation
			 * of the unbound_list as the process manager also needs to do it
			 */
			uint32_t new_alarm_id = 0;
			for (uint32_t i = 0 ; i < alarm_list_size ; i++)
			{
				if ( !alarm_list_t_get(am->alarm_list, i, &tmp) )
				{
					new_alarm_id = i;
					break;
				}
			}

			tinker_time_t alarm_time;
			tinker_time_add(&now, timeout, &alarm_time);
			alarm_t * const new_alarm = alarm_create(
					am->pool,
					new_alarm_id,
					&alarm_time,
					call_back,
					usr_data);
			if (new_alarm)
			{
				if (alarm_list_t_add(am->alarm_list, new_alarm))
				{
					alarm_calculate_next_alarm(am, new_alarm);
					if (alarm_id)
					{
						debug_print(ALARM, "Alarms: New alarm id is %d\n", new_alarm_id);
						*alarm_id = new_alarm_id;
					}
					if (am->next_alarm)
					{
						alarm_enable_timer(am);
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
error_t alarm_unset_alarm(
		alarm_manager_t * const am,
		const uint32_t alarm_id)
{
	error_t ret = NO_ERROR;
	debug_print(ALARM, "Alarms: Unset alarm id %d\n", alarm_id);
	alarm_t * alarm = NULL;
	const bool_t got = alarm_list_t_get(am->alarm_list, alarm_id, &alarm);
	if (got && alarm)
	{
		debug_print(ALARM, "Alarms: Removing %d\n", alarm_id);
		alarm_list_t_remove_item(am->alarm_list, alarm);
		if (alarm == am->next_alarm)
		{
			debug_prints(ALARM, "Alarms: Disabling the alarm timer\n");
			alarm_disable_timer(am);
			am->next_alarm = NULL;
			debug_prints(ALARM, "Alarms: Calculating next timer\n");
			alarm_calculate_next_alarm(am, NULL);
			if (am->next_alarm)
			{
				debug_prints(ALARM, "Alarms: Re-enabling the timer\n");
				alarm_enable_timer(am);
			}
		}
		debug_prints(ALARM, "Alarms: Deleting timer\n");
		alarm_delete(alarm);
	}
	else
	{
		ret = ALARM_ID_UNKNOWN;
	}
	debug_print(ALARM, "Alarms: Removing %d result %d\n", alarm_id, ret);
	return ret;
}

void alarm_calculate_next_alarm(
		alarm_manager_t * const am,
		alarm_t * const new_alarm)
{
	if (new_alarm)
	{
		if (am->next_alarm == NULL)
		{
			am->next_alarm = new_alarm;
		}
		else if (tinker_time_lt(
				alarm_get_time(new_alarm),
				alarm_get_time(am->next_alarm)))
		{
			am->next_alarm = new_alarm;
		}
	}
	else
	{
		if (alarm_list_t_size(am->alarm_list) > 0)
		{
			alarm_list_t_get(am->alarm_list, 0, &am->next_alarm);
			alarm_enable_timer(am);
		}
	}
}

static void alarm_handle_timer_timeout(tgt_context_t * const context, void * const param)
{
	(void)context; // UNUSED
	alarm_manager_t * const am = (alarm_manager_t*)param;
	if (am && am->next_alarm)
	{
		debug_prints(ALARM, "Alarms: An alarm needs to fire\n");
		alarm_fire_callback(am);
		alarm_calculate_next_alarm(am, NULL);
	}
}

void alarm_enable_timer(alarm_manager_t * const am)
{
	if (am->alarm_timer && am->next_alarm)
	{
		am->alarm_timer->timer_setup(
				am->alarm_timer->usr_data,	// user data for timer, not this alarm
				alarm_get_time(am->next_alarm),
				alarm_handle_timer_timeout,
				am);
	}
}

void alarm_disable_timer(alarm_manager_t * const am)
{
	if (am->alarm_timer)
	{
		am->alarm_timer->timer_cancel(am->alarm_timer->usr_data);
	}
}

alarm_t * alarm_create(
		mem_pool_info_t * const pool,
		const uint32_t id,
		tinker_time_t * const alarm_time,
		alarm_call_back * const callback,
		const alarm_user_data_t user_data)
{
	alarm_t * const alarm = (alarm_t*)mem_alloc(pool, sizeof(alarm_t));
	if (alarm)
	{
		alarm->id = id;
		alarm->alarm_time = *alarm_time;
		alarm->call_back = callback;
		alarm->usr_data = user_data;
		alarm->pool = pool;
	}
	return alarm;
}

void alarm_delete(alarm_t * const alarm)
{
	if (alarm)
	{
		mem_free(alarm->pool, alarm);
	}
}

const tinker_time_t* alarm_get_time(const alarm_t * const alarm)
{
	const tinker_time_t * t = NULL;
	if (alarm)
	{
		t = &alarm->alarm_time;
	}
	return t;
}

void alarm_fire_callback(alarm_manager_t * const am)
{
	debug_print(ALARM, "Alarms: Alarm Id %d calling %x\n", am->next_alarm->id, am->next_alarm->call_back);
	am->next_alarm->call_back(am, am->next_alarm->id, am->next_alarm->usr_data);
}

