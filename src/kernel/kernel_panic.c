/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
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
