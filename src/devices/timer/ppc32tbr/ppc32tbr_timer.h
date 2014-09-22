/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PPC32TBR_TIMER_H_
#define PPC32TBR_TIMER_H_

#include "kernel/process/process.h"
#include "kernel/time/timer.h"

void ppc_get_timer(const process_t * const parent, timer_t * const timer);

void ppc_check_timer(timer_t * const timer, const tgt_context_t * const context);

#endif /* PPC32TBR_TIMER_H_ */
