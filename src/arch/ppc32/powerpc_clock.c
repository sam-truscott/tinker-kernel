/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "powerpc_clock.h"
#include "powerpc32.h"

static uint64_t ppc_get_timebase_time(void);

static uint32_t ppc_nanoseconds_per_tb_tick = 0;

static clock_device_t ppc_timebase_clock =
{
		ppc_get_timebase_time
};

void ppc_setup_timebase_details(const uint64_t * const clock_as_hz, const uint32_t ticks_per_clock)
{
	ppc_nanoseconds_per_tb_tick = ppc_get_ns_per_tb_tick(clock_as_hz, ticks_per_clock);
}

uint64_t ppc_get_timebase_time(void)
{
	return ppc_get_tbr() * ppc_nanoseconds_per_tb_tick;
}

void ppc_convert_time_to_tbr(const tinker_time_t * const t, uint64_t * const tbr)
{
	if (t && tbr)
	{
		(*tbr) = (((t->seconds * ONE_SECOND_AS_NANOSECONDS) + t->nanoseconds)
				/ ppc_nanoseconds_per_tb_tick);
	}
}

clock_device_t * ppc_get_ppc_timebase_clock(void)
{
	return &ppc_timebase_clock;
}

