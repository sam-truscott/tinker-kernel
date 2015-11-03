/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef BCM2835_TIMER_H_
#define BCM2835_TIMER_H_

#include "kernel/process/process.h"
#include "kernel/time/alarm_manager.h"

void bcm2835_get_timer(mem_pool_info_t * const pool, timer_t * const timer, void * const base, const uint8_t instance);

clock_device_t * bcm2835_get_clock(void * const base_address, mem_pool_info_t * const pool);

#endif /* BCM2835_TIMER_H_ */
