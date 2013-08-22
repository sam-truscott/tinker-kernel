/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arch/tgt_types.h"

#ifndef PPC32_GENERIC_BSP_H_
#define PPC32_GENERIC_BSP_H_

/**
 * Decrementer Interrupt by the PowerPC
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
void __bsp_decrementer_interrupt(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

/**
 * All program error interrupts
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
void __bsp_fatal_program_error(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

/**
 * Requests from programs to perform system operations
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
void __bsp_system_call_request(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

#endif /* PPC32_GENERIC_BSP_H_ */
