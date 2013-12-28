/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OPIC_INTC_H_
#define OPIC_INTC_H_

#include "kernel/interrupts/intc.h"

__intc_t* __opic_intc_create(
		__mem_pool_info_t * const pool,
		const uint8_t * const base_address);

#endif /* OPIC_INTC_H_ */
