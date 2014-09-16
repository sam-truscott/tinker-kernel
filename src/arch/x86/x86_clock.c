/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "x86_clock.h"

static uint64_t x86_get_time(void * const user_data);

static clock_device_t x86_timebase_clock =
{
		.get_time = x86_get_time,
		.user_data = 0
};

static inline uint64_t x86_rdtsc(void)
{
    uint64_t ret;
    asm volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}

uint64_t x86_get_time(void * const user_data)
{
	(void)user_data;
	return x86_rdtsc();
}

clock_device_t * x86_get_ppc_timebase_clock(void)
{
	return &x86_timebase_clock;
}

