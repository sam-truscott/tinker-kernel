/*
 *
 * TINKER Source Code
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
		const uint32_t vector,
		__tgt_context_t * const context);

/**
 * All program error interrupts
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
void __bsp_fatal_program_error(
		const uint32_t vector,
		__tgt_context_t * const context);

/**
 * Requests from programs to perform system operations
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
void __bsp_system_call_request(
		const uint32_t vector,
		__tgt_context_t * const context);

#endif /* PPC32_GENERIC_BSP_H_ */
