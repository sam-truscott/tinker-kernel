/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_main.h"

#include "arch/tgt.h"
#include "arch/board_support.h"
#include "kernel_assert.h"
#include "kernel_initialise.h"
#include "kernel_version.h"
#include "kernel/console/print_out.h"
#include "kernel/shell/kshell.h"
#include "kernel/process/process_manager.h"
#include "kernel_in.h"

extern void kmain(void);

#include "arch/arm/arm_mmu.h"

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

	kernel_in_initialise();

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

	char * const buffer = mem_alloc_aligned(mem_get_default_pool(), 4096, 4096);
	debug_print("buffer allocated at 0x%X\n", buffer);
	mem_section_t * const kernel = mem_sec_create(
				mem_get_default_pool(),
				0,
				0,
				128 * 1024 * 1024,
				MMU_RANDOM_ACCESS_MEMORY,
				MMU_KERNEL_ACCESS,
				MMU_READ_WRITE);
		debug_print("kernel section allocated at 0x%X\n", kernel);

	mem_section_t * const section = mem_sec_create(
			mem_get_default_pool(),
			(uint32_t)buffer,
			0xc0000000,
			4096,
			MMU_RANDOM_ACCESS_MEMORY,
			MMU_KERNEL_ACCESS,
			MMU_READ_WRITE);
	debug_print("buffer section allocated at 0x%X\n", section);
	tgt_map_memory(kernel_get_process(), kernel);
	tgt_map_memory(kernel_get_process(), section);
	arm_set_translation_table_base(true, (tgt_pg_tbl_t*)process_get_page_table(kernel_get_process()));

	arm_enable_mmu();
	util_memset(buffer, 0, 4096);
	char * const buffer2 = (char*)0xc0000000;
	(void)buffer2;
	const uint32_t x = *(uint32_t*)(buffer2);
	(void)x;
	tgt_unmap_memory(kernel_get_process(), section);
	mem_sec_delete(section);
	mem_free(mem_get_default_pool(), buffer);

#if defined(KERNEL_DEBUGGING)
	debug_print("System: Calling kmain()\n");
#endif
	kmain();
#if defined(KERNEL_DEBUGGING)
	debug_print("System: Called kmain()\n");
#endif

#if defined (KERNEL_SHELL)
#if defined (KERNEL_DEBUGGING)
	debug_print("System: Creating kshell\n");
#endif
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

#if defined (KERNEL_DEBUGGING)
	debug_print("System: Entering User mode\n");
#endif
	tgt_enter_usermode();
#if defined (KERNEL_DEBUGGING)
	debug_print("System: Loading thread\n");
#endif
	TINKER_API_CALL_0(SYSCALL_LOAD_THREAD);
}
