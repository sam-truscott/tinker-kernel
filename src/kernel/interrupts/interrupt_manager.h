/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef INTERRUPT_MANAGER_H_
#define INTERRUPT_MANAGER_H_

#include "kernel/devices/device_type.h"
#include "intc.h"

 void __int_install_isr(const __intc_t * const intc);

 error_t __int_handle_external_vector(void);

/**
 * Interrupt that occurs when a the BSP requests a context
 * switch and the scheduler is executed
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void __int_context_switch_interrupt(
		__tgt_context_t * const context);

/**
 * Interrupt that occurs when the executing program/process/thread
 * incurs a fatal error and must be terminated
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void __int_fatal_program_error_interrupt(
		__tgt_context_t * const context);

/**
 * Interrupt that occurs when the executing program/process/thread
 * requests the kernel to perform a system function
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void __int_syscall_request_interrupt(
		__tgt_context_t * const context);

#endif /* INTERRUPT_MANAGER_H_ */
