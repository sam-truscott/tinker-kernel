/*
 *
 * SOS Source Code
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
#include "../arch/tgt.h"
#include "debug/debug_print.h"
#include "kernel/shell/kshell.h"
#include "kernel/process/process_manager.h"

extern void kmain(void);

static bool __kernel_first_run_var = true;

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
	__thread_t * idle_thread = __kernel_get_idle_thread();
	__kernel_assert("Kernel couldn't start Idle Thread", idle_thread != NULL);
	__sch_set_current_thread(idle_thread);

	kmain();

#ifndef HAS_CONSOLE
#define HAS_CONSOLE
#endif

#if defined ( HAS_CONSOLE )
	__proc_create_thread(
			idle_thread->parent, /* kernel process */
			"kshell",
			__kshell_start,
			1,
			0x1000,
			0,
			NULL,
			NULL);
#endif /* HAS_CONSOLE */

	/*
	 * these two need to be as close as possible
	 */
	SOS_API_CALL_0(syscall_load_thread);
}

bool __kernel_is_first_run(void)
{
	return __kernel_first_run_var;
}

void __kernel_first_run_ok(void)
{
	__kernel_first_run_var = false;
}

