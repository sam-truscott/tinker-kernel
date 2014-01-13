/*
 *
 * SOS Source Code
 * __________________
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

UNBOUNDED_LIST_TYPE(__alarm_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(__alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_ADD(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_REMOVE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_REMOVE_ITEM(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_GET(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_SPEC_SIZE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_CREATE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_ADD(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_REMOVE(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_GET(static, __alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY_SIZE(static, __alarm_list_t, __alarm_t*)

static void __alarm_calculate_next_alarm(__alarm_t * const new_alarm);

static void __alarm_enable_timer(void);

static void __alarm_disable_timer(void);

static void __alarm_handle_timer_timeout(void);

static __alarm_list_t * __alarm_list;

static __alarm_t * __alarm_next_alarm;

static __timer_t * __alarm_timer;

void __alarm_initialse(__mem_pool_info_t * const pool)
{
	__alarm_timer = NULL;
	__alarm_next_alarm = NULL;
	__alarm_list = __alarm_list_t_create(pool);
}

void __alarm_set_timer(__timer_t * const timer)
{
	__alarm_timer = timer;
}

error_t __alarm_set_alarm(
		__mem_pool_info_t * const pool,
		const sos_time_t * const timeout,
		__alarm_call_back * const call_back,
		const __alarm_user_data_t const usr_data,
		uint32_t * const alarm_id)
{
	error_t ret = NO_ERROR;
	if (pool && timeout)
	{
		const sos_time_t now = __time_get_system_time();

		/* check there's room of the new alarm */
		const uint32_t alarm_list_size = __alarm_list_t_size(__alarm_list);
		if (alarm_list_size < __MAX_ALARMS)
		{
			__alarm_t * tmp = NULL;
			/* find the next valid alarm ID TODO this should be an operation
			 * of the unbound_list as the process manager also needs to do it
			 */
			uint32_t new_alarm_id = 0;
			for (uint32_t i = 0 ; i < alarm_list_size ; i++)
			{
				if ( !__alarm_list_t_get(__alarm_list, i, &tmp) )
				{
					new_alarm_id = i;
					break;
				}
			}

			__alarm_t * const new_alarm = __alarm_create(
					pool,
					new_alarm_id,
					sos_time_add(&now, timeout),
					call_back,
					usr_data);
			if (new_alarm)
			{
				if (__alarm_list_t_add(__alarm_list, new_alarm))
				{
					__alarm_calculate_next_alarm(new_alarm);
					if (alarm_id)
					{
						*alarm_id = new_alarm_id;
					}
					if (__alarm_next_alarm)
					{
						__alarm_enable_timer();
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
error_t __alarm_unset_alarm(const uint32_t alarm_id)
{
	error_t ret = NO_ERROR;

	__alarm_t * alarm = NULL;
	const bool_t got = __alarm_list_t_get(__alarm_list, alarm_id, &alarm);
	if (got && alarm)
	{
		__alarm_list_t_remove_item(__alarm_list, alarm);
		if (alarm == __alarm_next_alarm)
		{
			__alarm_disable_timer();
			__alarm_next_alarm = NULL;
			__alarm_calculate_next_alarm(NULL);
			__alarm_enable_timer();
		}
		__alarm_delete(alarm);
	}
	else
	{
		ret = ALARM_ID_UNKNOWN;
	}

	return ret;
}

void __alarm_calculate_next_alarm(__alarm_t * const new_alarm)
{
	if (new_alarm)
	{
		if ( __alarm_next_alarm == NULL )
		{
			__alarm_next_alarm = new_alarm;
			__alarm_enable_timer();
		}
		else if (sos_time_lt(
				__alarm_get_time(new_alarm),
				__alarm_get_time(__alarm_next_alarm)))
		{
			__alarm_next_alarm = new_alarm;
			__alarm_enable_timer();
		}
	}
	else
	{
		if (__alarm_list_t_size(__alarm_list) > 0)
		{
			__alarm_list_t_get(__alarm_list, 0, &__alarm_next_alarm);
			__alarm_enable_timer();
		}
	}
}

void __alarm_handle_timer_timeout(void)
{
	if (__alarm_next_alarm)
	{
		__alarm_fire_callback(__alarm_next_alarm);
		__alarm_calculate_next_alarm(NULL);
	}
}

void __alarm_enable_timer(void)
{
	if (__alarm_timer && __alarm_next_alarm)
	{
		__alarm_timer->timer_setup(
				__alarm_timer->usr_data,
				__alarm_get_time(__alarm_next_alarm),
				__alarm_handle_timer_timeout);
	}
}

void __alarm_disable_timer(void)
{
	if (__alarm_timer)
	{
		__alarm_timer->timer_cancel(__alarm_timer->usr_data);
	}
}
