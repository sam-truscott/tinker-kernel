/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_main.h"

#include "../arch/board_support.h"
#include "kernel_initialise.h"
#include "kernel_version.h"
#include "kernel_assert.h"
#include "arch/tgt.h"
#include "console/print_out.h"
#include "kernel/shell/kshell.h"
#include "kernel/process/process_manager.h"

extern void kmain(void);

void __kernel_main(void)
{
	/*
	 * The BSP will initialise everything
	 */
	__bsp_initialise();

	/*
	 * Initialise the kernel into a known state;
	 */
	__debug_print("Kernel: Initialising...\n");
	__kernel_initialise();
	__debug_print("Kernel: Initialised OK.\n");

	/*
	 * Get the BSP to configure itself
	 */
	__debug_print("BSP: Setting up the Board...\n");
	__bsp_setup();
	__debug_print("BSP: Setup Complete\n");

	/*
	 * Start up message(s)
	 */
	__debug_print("\nSystem: Up - Kernel Version: %s, Build: %d\n\n", __KERNEL_VERSION, __KERNEL_BUILD);

	/*
	 * Show time; we're ready
	 *
	 * Sit in a holding pattern waiting for
	 * the scheduler to take over
	 *
	 * This will also perform the first call to initialise
	 * the timer
	 */
	__thread_t * const idle_thread = __kernel_get_idle_thread();
	__kernel_assert("Kernel couldn't start Idle Thread", idle_thread != NULL);
	__sch_set_current_thread(idle_thread);

	kmain();

#if defined (__KERNEL_SHELL1)
	__proc_create_thread(
			__thread_get_parent(idle_thread),
			"kshell",
			__kshell_start,
			1,
			0x1000,
			0,
			NULL,
			NULL);
#endif /* HAS_CONSOLE */

	__tgt_enter_usermode();
	TINKER_API_CALL_0(SYSCALL_LOAD_THREAD);
}


