/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "time/time_manager.h"

#include "kernel_assert.h"
#include "time.h"
#include "console/print_out.h"

typedef struct time_manager_t
{
	clock_device_t * time_system_clock;
} time_manager_t;

time_manager_t * time_initialise(mem_pool_info_t * const pool)
{
	time_manager_t * const tm = mem_alloc(pool, sizeof(time_manager_t));
	if (tm)
	{
		tm->time_system_clock = NULL;
		print_set_time_manager(tm);
	}
	return tm;
}

void time_set_system_clock(time_manager_t * const tm, clock_device_t * const device)
{
	if (tm)
	{
		tm->time_system_clock = device;
	}
}

void time_get_system_time(time_manager_t * const tm, tinker_time_t * const time)
{
	if (tm && tm->time_system_clock && time)
	{
		uint64_t time_system_time_ns = tm->time_system_clock->get_time(tm->time_system_clock->user_data);
		time->seconds = time_system_time_ns / ONE_SECOND_AS_NANOSECONDS;
		time->nanoseconds = (int64_t)(time_system_time_ns - ((int64_t)time->seconds * ONE_SECOND_AS_NANOSECONDS));
	}
}
