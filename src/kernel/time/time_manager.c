/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "time_manager.h"

#include "kernel/kernel_assert.h"
#include "time.h"

static uint64_t time_system_time_ns;
static clock_device_t * time_system_clock;

void time_initialise(void)
{
	time_system_time_ns = 0;
	time_system_clock = NULL;
}

void time_set_system_clock(clock_device_t * const device)
{
	time_system_clock = device;
}

void time_get_system_time(tinker_time_t * const time)
{
	if (time_system_clock && time)
	{
		time_system_time_ns = time_system_clock->get_time();
		time->seconds = time_system_time_ns / ONE_SECOND_AS_NANOSECONDS;
		time->nanoseconds = (int64_t)(time_system_time_ns - ((int64_t)time->seconds * ONE_SECOND_AS_NANOSECONDS));
	}
}
