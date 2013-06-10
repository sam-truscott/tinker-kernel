/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
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

#ifndef INTERRUPT_MANAGER_H_
#define INTERRUPT_MANAGER_H_

#include "../kernel_types.h"

/**
 * Initialise the Interrupt Services
 */
 void __int_initialise(void);

 void __int_install_isr(uint32_t vector, __kernel_device_t * device);

 void __int_uninstall_isr(uint32_t vector);

 error_t __int_handle_external_vector(uint32_t vector);

/**
 * Interrupt that occurs when a the BSP requests a context
 * switch and the scheduler is executed
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void __int_context_switch_interrupt(
		void * context,
		uint32_t context_size);

/**
 * Interrupt that occurs when the executing program/process/thread
 * incurs a fatal error and must be terminated
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void __int_fatal_program_error_interrupt(
		void * context,
		uint32_t context_size);

/**
 * Interrupt that occurs when the executing program/process/thread
 * requests the kernel to perform a system function
 * @param vector The interrupt vector from the processor
 * @param context The saved execution context
 * @param context_size The size of the context data
 */
void __int_syscall_request_interrupt(
		void * context,
		uint32_t context_size);

#endif /* INTERRUPT_MANAGER_H_ */
