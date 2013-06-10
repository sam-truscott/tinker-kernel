/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "alarm_manager.h"

#include "../memory/memory_manager.h"
#include "../time/time_manager.h"
#include "../time/time_utilities.h"
#include "../utils/collections/unbounded_list.h"

typedef struct
{
	__alarm_call_back *	call_back;
	void *				usr_data;
	uint32_t				usr_data_size;
	__time_t			alarm_time;
} __alarm_t;

UNBOUNDED_LIST_SPEC(static, alarm_list_t, __alarm_t*)
UNBOUNDED_LIST_BODY(static, alarm_list_t, __alarm_t*)

static void __alarm_calculate_next_alarm(__alarm_t * new_alarm);

static void __alarm_enable_timer(void);

static void __alarm_disable_timer(void);

static void __alarm_handle_timer_timeout(void);

static alarm_list_t __alarm_list;

static __alarm_t * __alarm_next_alarm;

static __timer_t * __alarm_timer;

void __alarm_initialse(void)
{
	__alarm_timer = NULL;
	__alarm_next_alarm = NULL;
	alarm_list_t_initialise(&__alarm_list, NULL);
}

void __alarm_set_timer(__timer_t * timer)
{
	__alarm_timer = timer;
}

error_t __alarm_set_alarm(
		__thread_t * thread,
		__time_t * timeout,
		__alarm_call_back * call_back,
		void * usr_data,
		uint32_t usr_data_size,
		uint32_t * alarm_id)
{
	error_t ret = NO_ERROR;
	if ( thread && timeout )
	{
		const __time_t now = __time_get_system_time();

		alarm_list_t * list = &__alarm_list;
		/* check there's room of the new alarm */
		const uint32_t alarm_list_size = alarm_list_t_size(list);
		if ( alarm_list_size < __MAX_ALARMS )
		{
			__alarm_t * tmp = NULL;
			/* find the next valid alarm ID TODO this should be an operation
			 * of the unbound_list as the process manager also needs to do it
			 */
			uint32_t new_alarm_id = ((alarm_list_size == 0) ? 1 : 0);
			for ( uint32_t i = 0 ; i < alarm_list_size ; i++ )
			{
				if ( !alarm_list_t_get(list, i, &tmp) )
				{
					new_alarm_id = i;
					break;
				}
			}

			__alarm_t * new_alarm = __mem_alloc(thread->parent->memory_pool, sizeof(__alarm_t));
			if ( new_alarm )
			{
				new_alarm->alarm_time = __time_add(now, *timeout);
				new_alarm->call_back = call_back;
				new_alarm->usr_data = usr_data;
				new_alarm->usr_data_size = usr_data_size;
				if ( alarm_list_t_add(list, new_alarm) )
				{
					__alarm_calculate_next_alarm(new_alarm);
					if ( alarm_id )
					{
						*alarm_id = new_alarm_id;
					}
					if ( __alarm_next_alarm )
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
error_t __alarm_unset_alarm(uint32_t alarm_id)
{
	error_t ret = NO_ERROR;

	__alarm_t * alarm = NULL;
	alarm_list_t_get(&__alarm_list, alarm_id, &alarm);
	if ( alarm )
	{
		alarm_list_t_remove_item(&__alarm_list, alarm);
		if ( alarm == __alarm_next_alarm)
		{
			__alarm_disable_timer();
			__alarm_next_alarm = NULL;
			__alarm_calculate_next_alarm(NULL);
			__alarm_enable_timer();
		}
	}
	else
	{
		ret = ALARM_ID_UNKNOWN;
	}

	return ret;
}

void __alarm_calculate_next_alarm(__alarm_t * new_alarm)
{
	if ( new_alarm )
	{
		if ( __alarm_next_alarm == NULL )
		{
			__alarm_next_alarm = new_alarm;
			__alarm_enable_timer();
		}
		else if (__time_lt( new_alarm->alarm_time, __alarm_next_alarm->alarm_time ) )
		{
			__alarm_next_alarm = new_alarm;
			__alarm_enable_timer();
		}
	}
	else
	{
		if ( alarm_list_t_size(&__alarm_list) > 0 )
		{
			alarm_list_t_get(&__alarm_list, 0, &__alarm_next_alarm);
			__alarm_enable_timer();
		}
	}
}

void __alarm_handle_timer_timeout(void)
{
	if ( __alarm_next_alarm )
	{
		__alarm_next_alarm->call_back(
				__alarm_next_alarm->usr_data,
				__alarm_next_alarm->usr_data_size);

		__alarm_calculate_next_alarm(NULL);
	}
}

void __alarm_enable_timer(void)
{
	if ( __alarm_timer && __alarm_next_alarm)
	{
		__alarm_timer->timer_setup(
				__alarm_timer->usr_data,
				__alarm_next_alarm->alarm_time,
				__alarm_handle_timer_timeout);
	}
}

void __alarm_disable_timer(void)
{
	if ( __alarm_timer )
	{
		__alarm_timer->timer_cancel(__alarm_next_alarm->usr_data);
	}
}
