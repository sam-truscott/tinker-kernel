/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PPC32TBR_TIMER_H_
#define PPC32TBR_TIMER_H_

#include "kernel/kernel_types.h"
#include "kernel/process/process.h"
#include "kernel/time/time.h"

void __ppc_get_timer(__process_t * parent, __timer_t * timer);

void __ppc_check_timer(__timer_t * timer);

#endif /* PPC32TBR_TIMER_H_ */
