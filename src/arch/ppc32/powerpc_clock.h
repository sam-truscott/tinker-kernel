/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef POWERPC_CLOCK_H_
#define POWERPC_CLOCK_H_

#include "kernel/time/time.h"

void __ppc_setup_timebase_details(uint64_t clock_as_hz, uint32_t ticks_per_clock);

__clock_device_t * __ppc_get_ppc_timebase_clock(void);

#endif /* POWERPC_CLOCK_H_ */
