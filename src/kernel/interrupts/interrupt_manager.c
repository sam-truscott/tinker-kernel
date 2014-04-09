/*
 *
 * TINKER Source Code
 * __________________
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

const __intc_t * __interrupt_manager_root = NULL;

void __int_install_isr(const __intc_t * const intc)
{
	__interrupt_manager_root = intc;
}

error_t __int_handle_external_vector(void)
{
	return __intc_handle(__interrupt_manager_root);
}

void __int_context_switch_interrupt(
		__tgt_context_t * const context)
{
	__kernel_assert("Context Switch Context missing", context != NULL);

	/* Copy over the context for the scheduler */
	__sch_set_context_for_next_thread(context);
	__bsp_enable_schedule_timer();
}

void __int_fatal_program_error_interrupt(
		__tgt_context_t * const context)
{
	__kernel_assert("Fatal Interrupt Context missing", context != NULL);

	__thread_t * const current = __sch_get_current_thread();
	__error_print("Fatal Fault: process: %s\t thread: %s\n",
			__process_get_image(__thread_get_parent(current)),
			__thread_get_name(current));
	__print_stack_trace(__tgt_get_context_stack_pointer(context));
	__sch_terminate_current_thread(context);
	__sch_set_context_for_next_thread(context);
}

void __int_syscall_request_interrupt(
		__tgt_context_t * const context)
{
	__kernel_assert("System Call Interrupt Context missing", context != NULL);

	__syscall_handle_system_call(context);
}
