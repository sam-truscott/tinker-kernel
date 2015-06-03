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

#include "kernel/syscall/syscall_handler.h"
#include "kernel/devices/kernel_intc_device.h"
#include "kernel/scheduler/scheduler.h"
#include "intc.h"

typedef struct interrupt_controller_t interrupt_controller_t;

interrupt_controller_t * int_create(
		mem_pool_info_t * const pool,
		syscall_handler_t * const syscall_handler,
		scheduler_t * const scheduler);

void int_install_isr(interrupt_controller_t * const intc, const intc_t * const rintc);

error_t int_handle_external_vector(interrupt_controller_t * const intc, tgt_context_t * const context);

void int_context_switch_interrupt(
		interrupt_controller_t * const intc,
		tgt_context_t * const context);

void int_fatal_program_error_interrupt(
		interrupt_controller_t * const intc,
		tgt_context_t * const context);

void int_syscall_request_interrupt(
		interrupt_controller_t * const intc,
		tgt_context_t * const context);

#endif /* INTERRUPT_MANAGER_H_ */
