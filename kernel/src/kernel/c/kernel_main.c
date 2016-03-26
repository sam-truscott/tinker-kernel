/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_main.h"

#include "tgt.h"
#include "board_support.h"
#include "kernel_assert.h"
#include "kernel_initialise.h"
#include "kernel_version.h"
#include "console/print_out.h"
#include "shell/kshell.h"
#include "process/process_list.h"
#include "scheduler/scheduler.h"
#include "unit_tests.h"

void kernel_main(void)
{
	/*
	 * The BSP will initialise everything
	 */
	bsp_initialise();
	tgt_disable_external_interrupts();

	/*
	 * Initialise the kernel into a known state;
	 */
#if defined(KERNEL_INIT)
	debug_print("Kernel: Initialising...\n");
#endif /* KERNEL_INIT */
	kernel_initialise();
#if defined(KERNEL_INIT)
	debug_print("Kernel: Initialised OK.\n");
#endif /* KERNEL_INIT */

	/*
	 * Start up message(s)
	 */
#if defined(KERNEL_INIT)
	debug_print("System: Up - Kernel Version: %s\n", KERNEL_VERSION);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("System: Testing Syscall...\n");
#endif /* KERNEL_INIT */
	TINKER_API_CALL_7(
			SYSCALL_TEST,
			SYSCALL_TEST_1,
			SYSCALL_TEST_2,
			SYSCALL_TEST_3,
			SYSCALL_TEST_4,
			SYSCALL_TEST_5,
			SYSCALL_TEST_6,
			SYSCALL_TEST_7);
#if defined(KERNEL_INIT)
	debug_print("System: Syscall OK\n");
#endif /* KERNEL_INIT */

#if defined(UNIT_TESTS)
	run_unit_tests();
#endif

#if defined (KERNEL_INIT)
	debug_print("System: Entering User mode\n");
#endif /* KERNEL_INIT */
	tgt_enter_usermode();
#if defined (KERNEL_INIT)
	debug_print("System: Loading thread\n");
#endif /* KERNEL_INIT */
	TINKER_API_CALL_0(SYSCALL_LOAD_THREAD);
}
