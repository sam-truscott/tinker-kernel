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

#include "interrupt_manager.h"

#include "arch/board_support.h"
#include "kernel/kernel_assert.h"
#include "kernel/kernel_initialise.h"
#include "kernel/debug/debug_print.h"
#include "kernel/debug/debug_stack_trace.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/syscall/syscall_handler.h"
#include "kernel/utils/util_memcpy.h"
#include "arch/tgt.h"

static __kernel_device_t * __int_isrs[__MAX_ISRS];

void __int_initialise(void)
{
	for ( uint32_t i = 0 ; i < __MAX_ISRS ; i++ )
	{
		__int_isrs[i] = NULL;
	}
}

void __int_install_isr(uint32_t vector, __kernel_device_t * device)
{
	__int_isrs[vector] = device;
}

void __int_uninstall_isr(uint32_t vector)
{
	__int_isrs[vector] = NULL;
}

error_t __int_handle_external_vector(uint32_t vector)
{
	error_t ret = NO_ERROR;
	__kernel_device_t * device = __int_isrs[vector];
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
		void * context,
		uint32_t context_size)
{
	__kernel_assert("Context Switch Context missing", context != NULL);
	__kernel_assert("Context Switch Context missing", context_size > 0);

	/*  Backup the vector data */
	__sch_save_context(context, context_size);

	/* Copy over the context for the scheduler */
	__sch_prepare_scheduler_context(context, context_size);
}

void __int_fatal_program_error_interrupt(
		void * context,
		uint32_t context_size)
{
	__kernel_assert("Fatal Interrupt Context missing", context != NULL);
	__kernel_assert("Fatal Interrupt Context missing", context_size > 0);

	__debug_print("Fatal Fault: Context @ %X (%d bytes)\n", context, context_size);
	__sch_terminate_current_thread(context, context_size);
	__sch_prepare_scheduler_context(context, context_size);
}

void __int_syscall_request_interrupt(
		void * context,
		uint32_t context_size)
{
	__kernel_assert("System Call Interrupt Context missing", context != NULL);
	__kernel_assert("System Call Interrupt Context missing", context_size > 0);

	__syscall_handle_system_call(context, context_size);
}
