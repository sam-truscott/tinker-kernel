/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OPIC_TIMER_H_
#define OPIC_TIMER_H_

#include "kernel/time/alarm_manager.h"

void opic_tmr_get_timer(mem_pool_info_t * const pool, uint32_t * base_address, timer_t * timer);

#endif /* OPIC_TIMER_H_ */
