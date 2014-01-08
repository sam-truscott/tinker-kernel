/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "alarm.h"
#include "time.h"

typedef struct __alarm_t
{
	uint32_t 			id;
	__alarm_call_back *	call_back;
	void * 				usr_data;
	__time_t			alarm_time;
} __alarm_internal_t;

__alarm_t * __alarm_create(
		__mem_pool_info_t * const pool,
		const uint32_t id,
		const __time_t alarm_time,
		__alarm_call_back * const callback,
		const __alarm_user_data_t user_data)
{
	__alarm_t * const alarm = (__alarm_t*)__mem_alloc(pool, sizeof(__alarm_t));
	if (alarm)
	{
		alarm->id = id;
		alarm->alarm_time = alarm_time;
		alarm->call_back = callback;
		alarm->usr_data = user_data;
	}
	return alarm;
}

__time_t __alarm_get_time(const __alarm_t * const alarm)
{
	__time_t t = __ZERO_TIME;
	if (alarm)
	{
		t = alarm->alarm_time;
	}
	return t;
}

void __alarm_fire_callback(const __alarm_t * const alarm)
{
	alarm->call_back(alarm->id, alarm->usr_data);
}
