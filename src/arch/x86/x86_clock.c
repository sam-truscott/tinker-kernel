/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "x86_clock.h"

static uint64_t x86_get_timebase_time(void);

static clock_device_t x86_timebase_clock =
{
		x86_get_timebase_time
};

uint64_t x86_get_timebase_time(void)
{
	return 1;
}

clock_device_t * x86_get_ppc_timebase_clock(void)
{
	return &x86_timebase_clock;
}

