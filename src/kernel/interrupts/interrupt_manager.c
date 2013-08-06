/*
 *
 * SOS Source Code
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

static const __kernel_device_t * __int_isrs[__MAX_ISRS];

void __int_initialise(void)
{
	for ( uint32_t i = 0 ; i < __MAX_ISRS ; i++ )
	{
		__int_isrs[i] = NULL;
	}
}

void __int_install_isr(
		 const uint32_t vector,
		 const __kernel_device_t * const device)
{
	__int_isrs[vector] = device;
}

void __int_uninstall_isr(const uint32_t vector)
{
	__int_isrs[vector] = NULL;
}

error_t __int_handle_external_vector(const uint32_t vector)
{
	error_t ret = NO_ERROR;
	const __kernel_device_t * const device = __int_isrs[vector];
	if ( device && device->isr )
	{
		device->isr(device->user_data, vector);
	}
	else
	{
		ret = UNKNOWN_EXTERNAL_INTERRUPT_VECTOR;
	}
	return ret;
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

	__error_print("Fatal Fault: Context @ %X\n", context);
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
