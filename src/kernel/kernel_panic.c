/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_panic.h"

#include "arch/board_support.h"
#include "console/stack_trace.h"
#include "console/print_out.h"
#include "arch/tgt.h"

void __kernel_panic(void)
{
	/*
	 * Disable interrupts to prevent the panic being stopped
	 */
	__tgt_disable_external_interrupts();

	__error_print("KERNEL PANIC\n");
	__print_current_stack_trace();
	volatile int crash = 0;

	/*
	 * Sit in a tight loop
	 * TODO talk to the BSP/Target
	 */
	while(crash!=1)
	{
		/*
		 * Sit here, quietly, waiting...
		 */
	}
}
