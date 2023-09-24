/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "interrupts/interrupt_manager.h"

#include "board_support.h"
#include "kernel_assert.h"
#include "console/print_out.h"
#include "console/stack_trace.h"
#include "scheduler/scheduler.h"
#include "syscall/syscall_handler.h"
#include "utils/util_memcpy.h"
#include "tgt.h"

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

return_t int_handle_external_vector(interrupt_controller_t * const intc, tgt_context_t * const context)
{
	if (intc)
	{
		intc->is_external = true;
		const thread_t * const current_thread = sch_get_current_thread(intc->scheduler);
		if (is_debug_enabled(INTC))
		{
			debug_print(INTC, "INTC: Thread before external vector: %s\n", thread_get_name(current_thread));
		}
		const return_t intc_result = intc_handle(intc->interrupt_manager_root, context);
		debug_prints(INTC, "INTC: Switching thread due to external interrupt\n");
		sch_set_context_for_next_thread(
				intc->scheduler, context,
				thread_get_state(current_thread));
		if (is_debug_enabled(INTC))
		{
			debug_print(INTC, "INTC: Thread after external vector: %s\n", thread_get_name(sch_get_current_thread(intc->scheduler)));
		}
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

static inline mem_t virtual_to_real(
		process_t * process,
		mem_t address)
{
	/* TODO the below code needs fixing */
	const bool_t is_kernel = process_is_kernel(process);
	mem_t result;
	if (is_kernel)
	{
		result = ((address >= VIRTUAL_ADDRESS_SPACE(is_kernel)) ? process_virt_to_real(process, address) : address);
	}
	else
	{
		result = process_virt_to_real(process, address);
	}
	return result;
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
#if defined (PRINT_STACK_ON_ERROR)
	// FIXME: this is broken due to a second exception being raised
	print_stack_trace(thread_get_parent(current), tgt_get_context_stack_pointer(context));
#endif /* PRINT_STACK_ON_ERROR */
	mem_t real_pc = virtual_to_real(thread_get_parent(current), tgt_get_pc(context));
	error_print("Real PC: %x\n", real_pc);
	sch_terminate_current_thread(intc->scheduler, context);
	sch_set_context_for_next_thread(
			intc->scheduler,
			context,
			THREAD_TERMINATED);

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
