/*
 *
 * TINKER Source Code
 * 
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

void kernel_main(void)
{
	/*
	 * The BSP will initialise everything
	 */
	bsp_initialise();

	/*
	 * Initialise the kernel into a known state;
	 */
	debug_print("Kernel: Initialising...\n");
	kernel_initialise();
	debug_print("Kernel: Initialised OK.\n");

	/*
	 * Get the BSP to configure itself
	 */
	debug_print("BSP: Setting up the Board...\n");
	bsp_setup();
	debug_print("BSP: Setup Complete\n");

	/*
	 * Start up message(s)
	 */
	debug_print("System: Up - Kernel Version: %s, Build: %d\n", KERNEL_VERSION, KERNEL_BUILD);

	/*
	 * Show time; we're ready
	 *
	 * Sit in a holding pattern waiting for
	 * the scheduler to take over
	 *
	 * This will also perform the first call to initialise
	 * the timer
	 */
	thread_t * const idle_thread = kernel_get_idle_thread();
	kernel_assert("Kernel couldn't start Idle Thread", idle_thread != NULL);
	sch_set_current_thread(idle_thread);

	kmain();

#if defined (KERNEL_SHELL1)
	proc_create_thread(
			thread_get_parent(idle_thread),
			"kshell",
			kshell_start,
			1,
			0x1000,
			0,
			NULL,
			NULL);
#endif /* HAS_CONSOLE */

	tgt_enter_usermode();
	TINKER_API_CALL_0(SYSCALL_LOAD_THREAD);
}
