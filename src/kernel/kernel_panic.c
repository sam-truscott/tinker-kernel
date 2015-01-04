/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_panic.h"

#include "arch/board_support.h"
#include "console/stack_trace.h"
#include "console/print_out.h"
#include "arch/tgt.h"

void kernel_panic(void)
{
	/*
	 * Disable interrupts to prevent the panic being stopped
	 */
	tgt_disable_external_interrupts();

	error_print("KERNEL PANIC\n");
	print_current_stack_trace();
	volatile int crash = 0;

	/*
	 * Sit in a tight loop
	 */
	while(crash!=1)
	{
		/*
		 * Sit here, quietly, waiting...
		 */
		tgt_wait_for_interrupt();
	}
}
