/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "interrupt_manager.h"

#include "arch/board_support.h"
#include "kernel/kernel_assert.h"
#include "kernel/kernel_initialise.h"
#include "kernel/console/print_out.h"
#include "kernel/console/stack_trace.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/syscall/syscall_handler.h"
#include "kernel/utils/util_memcpy.h"
#include "arch/tgt.h"

const intc_t * interrupt_manager_root = NULL;

void int_install_isr(const intc_t * const intc)
{
	interrupt_manager_root = intc;
}

error_t int_handle_external_vector(void)
{
	return intc_handle(interrupt_manager_root);
}

void int_context_switch_interrupt(
		tgt_context_t * const context)
{
	kernel_assert("Context Switch Context missing", context != NULL);

	/* Copy over the context for the scheduler */
	sch_set_context_for_next_thread(context);
	bsp_enable_schedule_timer();
}

void int_fatal_program_error_interrupt(
		tgt_context_t * const context)
{
	kernel_assert("Fatal Interrupt Context missing", context != NULL);

	thread_t * const current = sch_get_current_thread();
	error_print("Fatal Fault: process: %s\t thread: %s\n",
			process_get_image(thread_get_parent(current)),
			thread_get_name(current));
	print_stack_trace(tgt_get_context_stack_pointer(context));
	sch_terminate_current_thread(context);
	sch_set_context_for_next_thread(context);
}

void int_syscall_request_interrupt(
		tgt_context_t * const context)
{
	kernel_assert("System Call Interrupt Context missing", context != NULL);

	syscall_handle_system_call(context);
}
