/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "powerpc_clock.h"
#include "powerpc32.h"

static uint64_t __ppc_get_timebase_time(void);

static uint32_t __ppc_nanoseconds_per_tb_tick = 0;

static __clock_device_t __ppc_timebase_clock =
{
		__ppc_get_timebase_time
};

void __ppc_setup_timebase_details(uint64_t clock_as_hz, uint32_t ticks_per_clock)
{
	__ppc_nanoseconds_per_tb_tick = __ppc_get_ns_per_tb_tick(clock_as_hz, ticks_per_clock);
}

uint64_t __ppc_get_timebase_time(void)
{
	const uint64_t tbr = __ppc_get_tbr();
	return tbr * __ppc_nanoseconds_per_tb_tick;
}

__clock_device_t * __ppc_get_ppc_timebase_clock(void)
{
	return &__ppc_timebase_clock;
}

