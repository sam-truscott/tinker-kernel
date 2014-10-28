/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "alarm.h"

#include "kernel/time/time.h"
#include "kernel/console/print_out.h"

typedef struct alarm_t
{
	uint32_t 			id;
	alarm_call_back *	call_back;
	void * 				usr_data;
	tinker_time_t			alarm_time;
	mem_pool_info_t * pool;
} alarm_internal_t;

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

void alarm_fire_callback(const alarm_t * const alarm)
{
#if defined(ALARM_DEBUGGING)
	debug_print("Alarms: Alarm Id %d calling %x\n", alarm->id, alarm->call_back);
#endif
	alarm->call_back(alarm->id, alarm->usr_data);
}
