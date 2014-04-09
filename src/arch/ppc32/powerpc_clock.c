/*
 *
 * TINKER Source Code
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

void __ppc_setup_timebase_details(const uint64_t * const clock_as_hz, const uint32_t ticks_per_clock)
{
	__ppc_nanoseconds_per_tb_tick = __ppc_get_ns_per_tb_tick(clock_as_hz, ticks_per_clock);
}

uint64_t __ppc_get_timebase_time(void)
{
	return __ppc_get_tbr() * __ppc_nanoseconds_per_tb_tick;
}

void __ppc_convert_time_to_tbr(const tinker_time_t * const t, uint64_t * const tbr)
{
	if (t && tbr)
	{
		(*tbr) = (((t->seconds * ONE_SECOND_AS_NANOSECONDS) + t->nanoseconds)
				/ __ppc_nanoseconds_per_tb_tick);
	}
}

__clock_device_t * __ppc_get_ppc_timebase_clock(void)
{
	return &__ppc_timebase_clock;
}

