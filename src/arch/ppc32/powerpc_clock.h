/*************************************************************************
 *
 * SOS 3 Source Code
 * __________________
 *
 *  [2012] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef POWERPC_CLOCK_H_
#define POWERPC_CLOCK_H_

#include "kernel/kernel_types.h"

void __ppc_setup_timebase_details(uint64_t clock_as_hz, uint32_t ticks_per_clock);

__clock_device_t * __ppc_get_ppc_timebase_clock(void);

#endif /* POWERPC_CLOCK_H_ */
