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
#include "api/sos_api_time.h"

void __ppc_setup_timebase_details(const uint64_t * const clock_as_hz, const uint32_t ticks_per_clock);

void __ppc_convert_time_to_tbr(const sos_time_t * const t, uint64_t * const tbr);

__clock_device_t * __ppc_get_ppc_timebase_clock(void);

#endif /* POWERPC_CLOCK_H_ */
