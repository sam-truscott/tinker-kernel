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

static inline uint64_t x86_rdtsc(void)
{
    uint64_t ret;
    asm volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}

uint64_t x86_get_timebase_time(void)
{
	return x86_rdtsc();
}

clock_device_t * x86_get_ppc_timebase_clock(void)
{
	return &x86_timebase_clock;
}

