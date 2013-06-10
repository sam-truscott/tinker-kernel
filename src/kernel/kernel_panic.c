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

#include "kernel_panic.h"

#include "../arch/board_support.h"
#include "debug/debug_stack_trace.h"
#include "debug/debug_print.h"
#include "../arch/tgt.h"

void __kernel_panic(void)
{
	/*
	 * Disable interrupts to prevent the panic being stopped
	 */
	__tgt_disable_external_interrupts();

	__debug_print("KERNEL PANIC\n");
	__debug_print_current_stack_trace();

	/*
	 * Sit in a tight loop
	 * TODO talk to the BSP/Target
	 */
	while(1!=2)
	{
		/*
		 * Sit here, quietly, waiting...
		 */
	}
}
