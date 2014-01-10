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
#include "time.h"

static int64_t __time_system_time_ns;
static __clock_device_t * __time_system_clock;

void __time_initialise(void)
{
	__time_system_time_ns = 0;
	__time_system_clock = NULL;
}

void __time_set_system_clock(__clock_device_t * const device)
{
	__time_system_clock = device;
}

sos_time_t __time_get_system_time(void)
{
	sos_time_t time = {0,0};

	__kernel_assert("system clock isn't setup correctly", __time_system_clock != NULL);
	__time_system_time_ns = (int64_t)__time_system_clock->get_time();
	time.seconds = (int32_t)__time_system_time_ns / ONE_SECOND_AS_NANOSECONDS;
	time.nanoseconds = (int64_t)(__time_system_time_ns - ((int64_t)time.seconds * ONE_SECOND_AS_NANOSECONDS));

	return time;
}
