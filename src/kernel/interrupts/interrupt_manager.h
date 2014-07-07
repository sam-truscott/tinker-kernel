/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef INTERRUPT_MANAGER_H_
#define INTERRUPT_MANAGER_H_

#include "kernel/devices/kernel_intc_device.h"
#include "intc.h"

 void int_install_isr(const intc_t * const intc);

 error_t int_handle_external_vector(void);

/**
 * Interrupt that occurs when a the BSP requests a context
 * switch and the scheduler is executed
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void int_context_switch_interrupt(
		tgt_context_t * const context);

/**
 * Interrupt that occurs when the executing program/process/thread
 * incurs a fatal error and must be terminated
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void int_fatal_program_error_interrupt(
		tgt_context_t * const context);

/**
 * Interrupt that occurs when the executing program/process/thread
 * requests the kernel to perform a system function
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void int_syscall_request_interrupt(
		tgt_context_t * const context);

#endif /* INTERRUPT_MANAGER_H_ */
