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
#include "elf_example.h"
#include "loader/loader.h"

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
	debug_print("Kernel: Initialising...\n");
	kernel_initialise();
	debug_print("Kernel: Initialised OK.\n");

	/*
	 * Start up message(s)
	 */
	debug_print("System: Up - Kernel Version: %s\n", KERNEL_VERSION);

	// TODO move to unit tests
#if defined(SYSCALL_DEBUGGING)
	debug_print("System: Testing Syscall...\n");
#endif
	TINKER_API_CALL_7(
			SYSCALL_TEST,
			SYSCALL_TEST_1,
			SYSCALL_TEST_2,
			SYSCALL_TEST_3,
			SYSCALL_TEST_4,
			SYSCALL_TEST_5,
			SYSCALL_TEST_6,
			SYSCALL_TEST_7);
#if defined(SYSCALL_DEBUGGING)
	debug_print("System: Syscall OK\n");
#endif

#if defined(UNIT_TESTS)
	run_unit_tests();
#endif

#if defined (KERNEL_SHELL)
#if defined (KERNEL_DEBUGGING)
	debug_print("System: Creating kshell\n");
#endif
	proc_create_thread(
			kernel_get_process(),
			"kernel_shell",
			kshell_start,
			1,
			0x400,
			0,
			NULL,
			NULL);
#endif /* KERNEL_SHELL */

	load_elf(
			mem_get_default_pool(),
			kernel_get_proc_list(),
			elf_get_example());

#if defined (KERNEL_DEBUGGING)
	debug_print("System: Entering User mode\n");
#endif
	tgt_enter_usermode();
#if defined (KERNEL_DEBUGGING)
	debug_print("System: Loading thread\n");
#endif
	TINKER_API_CALL_0(SYSCALL_LOAD_THREAD);
}
