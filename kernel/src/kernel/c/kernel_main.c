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

#if defined(UNIT_TESTS)
extern void run_unit_tests(void);
#endif /* UNIT_TESTS */

void kernel_main(void)
{
	/*
	 * Initialise the kernel into a known state;
	 */
	debug_prints(INITIALISATION, "Kernel: Initialising...\n");
	kernel_initialise();
	debug_prints(INITIALISATION, "Kernel: Initialised OK.\n");

	/*
	 * Start up message(s)
	 */
	debug_print(INITIALISATION, "System: Up - Kernel Version: %s\n", KERNEL_VERSION);

	debug_prints(INITIALISATION, "System: Testing Syscall...\n");
	TINKER_API_CALL_7(
			SYSCALL_TEST,
			SYSCALL_TEST_1,
			SYSCALL_TEST_2,
			SYSCALL_TEST_3,
			SYSCALL_TEST_4,
			SYSCALL_TEST_5,
			SYSCALL_TEST_6,
			SYSCALL_TEST_7);
	debug_prints(INITIALISATION, "System: Syscall OK\n");

#if defined(UNIT_TESTS)
	run_unit_tests();
#endif

	debug_prints(INITIALISATION, "System: Entering User mode\n");
	tgt_enter_usermode();
	debug_prints(INITIALISATION, "System: Loading thread\n");
	TINKER_API_CALL_0(SYSCALL_LOAD_THREAD);
}
