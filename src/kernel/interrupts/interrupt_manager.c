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
#include "kernel/console/print_out.h"
#include "kernel/console/stack_trace.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/syscall/syscall_handler.h"
#include "kernel/utils/util_memcpy.h"
#include "arch/tgt.h"

typedef struct interrupt_controller_t
{
	const intc_t * interrupt_manager_root;
	volatile bool_t is_external;
	syscall_handler_t * handler;
	scheduler_t * scheduler;
} interrupt_controller_t;

interrupt_controller_t * int_create(
		mem_pool_info_t * const pool,
		syscall_handler_t * const syscall_handler,
		scheduler_t * const scheduler)
{
	interrupt_controller_t * const intc = mem_alloc(pool, sizeof(interrupt_controller_t));
	if (intc)
	{
		intc->interrupt_manager_root = NULL;
		intc->is_external = false;
		intc->handler = syscall_handler;
		intc->scheduler = scheduler;
	}
	return intc;
}

void int_install_isr(interrupt_controller_t * const intc, const intc_t * const rintc)
{
	if (intc)
	{
		intc->interrupt_manager_root = rintc;
	}
}

error_t int_handle_external_vector(interrupt_controller_t * const intc, tgt_context_t * const context)
{
	if (intc)
	{
		intc->is_external = true;
		const thread_t * const current_thread = sch_get_current_thread(intc->scheduler);
#if defined(INTC_DEBUGGING)
		debug_print("INTC: Thread before external vector: %s\n", thread_get_name(current_thread));
#endif
		const error_t intc_result = intc_handle(intc->interrupt_manager_root, context);
	#if defined(INTC_DEBUGGING)
		debug_print("INTC: Switching thread due to external interrupt\n");
	#endif
		sch_set_context_for_next_thread(
				intc->scheduler, context,
				thread_get_state(current_thread));
	#if defined(INTC_DEBUGGING)
		const thread_t * const new_thread = sch_get_current_thread(intc->scheduler);
		debug_print("INTC: Thread after external vector: %s\n", thread_get_name(new_thread));
	#endif
		intc->is_external = false;
		return intc_result;
	}
	else
	{
		error_print("INTC: Interrupt controller is null\n");
		return PARAMETERS_NULL;
	}
}

void int_context_switch_interrupt(
		interrupt_controller_t * const intc,
		tgt_context_t * const context)
{
	if (intc)
	{
		kernel_assert("Context Switch Context missing", context != NULL);
		/* Copy over the context for the scheduler */
		sch_set_context_for_next_thread(
				intc->scheduler, context,
				thread_get_state(sch_get_current_thread(intc->scheduler)));
	}
}

void int_fatal_program_error_interrupt(
		interrupt_controller_t * const intc,
		tgt_context_t * const context)
{
	kernel_assert("Fatal Interrupt Context missing", context != NULL);

	thread_t * const current = sch_get_current_thread(intc->scheduler);
	error_print("Fatal Fault: process: %s\t thread: %s\n",
			process_get_image(thread_get_parent(current)),
			thread_get_name(current));
	print_stack_trace(thread_get_parent(current), tgt_get_context_stack_pointer(context));
	sch_terminate_current_thread(intc->scheduler, context);
	sch_set_context_for_next_thread(
			intc->scheduler,
			context,
			thread_get_state(current));
}

void int_syscall_request_interrupt(
		interrupt_controller_t * const intc,
		tgt_context_t * const context)
{
	if (intc)
	{
		kernel_assert("System Call Interrupt Context missing", context != NULL);
		syscall_handle_system_call(intc->handler, context);
	}
}
