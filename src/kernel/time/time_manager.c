/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "time_manager.h"

#include "kernel/kernel_assert.h"
#include "time_utilities.h"
#include "time.h"

static int64_t __time_system_time_ns;
static __clock_device_t * __time_system_clock;

void __time_initialise(void)
{
	__time_system_time_ns = 0;
	__time_system_clock = NULL;
}

void __time_set_system_clock(__clock_device_t * device)
{
	__time_system_clock = device;
}

__time_t __time_get_system_time(void)
{
	__time_t time = {0,0};

	__kernel_assert("system clock isn't setup correctly", __time_system_clock != NULL);
	__time_system_time_ns = (int64_t)__time_system_clock->get_time();
	time.seconds = (int32_t)__time_system_time_ns / ONE_SECOND_AS_NANOSECONDS;
	time.nanoseconds = (int64_t)(__time_system_time_ns - ((int64_t)time.seconds * ONE_SECOND_AS_NANOSECONDS));

	return time;
}

void __time_sleep(__time_t period)
{
	const __time_t start = __time_get_system_time();
	const __time_t end = __time_add(start, period);
	while ( 1 )
	{
		const __time_t now = __time_get_system_time();
		if(__time_gt(now, end))
		{
			break;
		}
	}
}
