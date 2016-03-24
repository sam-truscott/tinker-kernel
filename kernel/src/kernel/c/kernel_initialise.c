/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_initialise.h"

#include "board_support.h"
#include "kernel_assert.h"
#include "kernel_idle.h"
#include "console/print_out.h"
#include "interrupts/interrupt_manager.h"
#include "memory/memory_manager.h"
#include "process/process_list.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"
#include "utils/util_memset.h"
#include "objects/registry.h"
#include "syscall/syscall_handler.h"
#include "scheduler/scheduler.h"
#include "shell/kshell.h"
#include "tgt.h"
#include "loader/loader.h"

void kernel_initialise(void)
{
	const uint32_t memory_start = bsp_get_usable_memory_start();
	const uint32_t memory_end = bsp_get_usable_memory_end();

	debug_print("Memory: Initialising Pool, start %X, end %x\n",
			memory_start,
			memory_end);

	kernel_assert("Memory End > 0", memory_end > 0);
	kernel_assert("Memory Size > 0", memory_end > memory_start);

	const bool_t mem_init_ok = mem_initialise(
			memory_start,
			memory_end);
	kernel_assert("Failed to Initialise Memory Manager", mem_init_ok);

	mem_pool_info_t * const pool = mem_get_default_pool();

	debug_print("Time: Initialising services...\n");
	time_manager_t * const time_manager = time_initialise(pool);
	alarm_manager_t * const alarm_manager = alarm_initialse(pool, time_manager);
	debug_print("Time: Time Manager at %x, Alarm Manager at %x\n", time_manager, alarm_manager);

	debug_print("Registry: Initialising the Registry...\n");
	registry_t * const registry = registry_create(pool);
	debug_print("Registry: Registry at %x\n", registry);

	debug_print("Scheduler: Initialising Scheduler...\n");
	scheduler_t * const scheduler = sch_create_scheduler(pool);
	debug_print("Scheduler: Scheduler at %x\n", scheduler);

	debug_print("Process: Initialising Management...\n");
	proc_list_t * const proc_list = proc_create(pool, scheduler, alarm_manager);
	debug_print("Process: Process list at %x\n", proc_list);

	debug_print("Loader: Initialising Loader...\n");
	loader_t * const loader = loader_create(pool, proc_list);

	debug_print("Syscall: Initialising...\n");
	syscall_handler_t * const syscall_handler = create_handler(
			pool,
			proc_list,
			registry,
			scheduler,
			time_manager,
			alarm_manager,
			loader);
	debug_print("Syscall: Syscall Handler at %x\n", syscall_handler);

	debug_print("Intc: Initialising Interrupt Controller...\n");
	interrupt_controller_t * const interrupt_controller = int_create(pool, syscall_handler, scheduler);
	debug_print("Intc: Initialising Interrupt Controller at %x\n", interrupt_controller);

	debug_print("Kernel: Initialising Kernel Process...\n");

	extern char * __text;
	extern char * __text_end;
	extern char * __data_end;
	char * text_pos = (char*)&__text;
	char * text_epos = (char*)&__text_end;
	char * data_pos = 0;
	char * data_end = (char*)&__data_end;
	tinker_mempart_t data =
	{
		.real = (uint32_t)data_pos,
		.virt = (uint32_t)data_pos,
		.size = (uint32_t)(data_end - data_pos),
		.mem_type = MEM_RANDOM_ACCESS_MEMORY,
		.priv = MEM_KERNEL_ACCESS,
		.access = MEM_READ_WRITE,
		.next = NULL
	};
	tinker_mempart_t code =
	{
		.real = (uint32_t)text_pos,
		.virt = (uint32_t)text_pos,
		.size = (uint32_t)(text_epos - text_pos),
		.mem_type = MEM_RANDOM_ACCESS_MEMORY,
		.priv = MEM_KERNEL_ACCESS,
		.access = MEM_READ_ONLY,
		.next = &data
	};
	tinker_meminfo_t meminfo =
	{
		.heap_size = KERNEL_HEAP,
		.stack_size = KERNEL_IDLE_STACK,
		.first_part = &code
	};
	debug_print("Kernel: Process list %x\n", proc_list);
	process_t * kernel_process = NULL;
	proc_create_process(
			proc_list,
			"kernel",
			"kernel_idle",
			kernel_idle,
			KERNEL_IDLE_PRIORITY,
			&meminfo,
			THREAD_FLAG_NONE,
			&kernel_process);

	kernel_device_init(kernel_process, registry, proc_list);

	proc_set_kernel_process(proc_list, kernel_process);
	thread_t * const kernel_idle_thread = process_get_main_thread(kernel_process);
	sch_set_kernel_idle_thread(scheduler, kernel_idle_thread);

	kernel_assert("Kernel Process not created",kernel_process != NULL);
	kernel_assert("Kernel Idle Thread not created", kernel_idle_thread != NULL);

	thread_set_state(kernel_idle_thread, THREAD_SYSTEM);

#if defined(KERNEL_SHELL)
	kshell_setup(pool, proc_list);
#endif

	// Map the RAM into Kernel space
	mem_section_t * const kernel_ram_sec = mem_sec_create(
			mem_get_default_pool(),
			4096,
			0,
			bsp_get_usable_memory_end(),
			MMU_RANDOM_ACCESS_MEMORY,
			MMU_KERNEL_ACCESS,
			MMU_READ_WRITE);
	tgt_map_memory(kernel_process, kernel_ram_sec);

	kernel_assert("Kernel couldn't start Idle Thread", kernel_idle_thread != NULL);
	sch_set_current_thread(scheduler, kernel_idle_thread);

	/* Get the BSP to configure itself */
	debug_print("BSP: Setting up the Board...\n");
	bsp_setup(interrupt_controller, time_manager, alarm_manager, kernel_process);
	debug_print("BSP: Setup Complete\n");

#if defined (KERNEL_SHELL)
#if defined (KERNEL_DEBUGGING)
	debug_print("System: Creating kshell\n");
#endif /* KERNEL_DEBUGGING */
	proc_create_thread(
			kernel_process,
			"kernel_shell",
			kshell_start,
			1,
			0x400,
			0,
			NULL,
			NULL);
#endif /* KERNEL_SHELL */

	extern uint32_t apps;
	uint32_t app_start = (uint32_t)&apps;
#if defined(ELF_LOAD_DEBUGGING)
	debug_print("Starting at %x\n", app_start);
#endif /* ELF_LOAD_DEBUGGING */
	uint32_t sz = *(uint32_t*)app_start;
	app_start += 4;
	while (sz)
	{
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Addr = %8x, Size = %x\n", app_start, sz);
#endif /* ELF_LOAD_DEBUGGING */
		error_t elf_error = load_elf(
				loader,
				(void*)app_start,
				"app",
				128,
				0);
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("ELF load result %d\n", elf_error);
#endif /* ELF_LOAD_DEBUGGING */
		app_start += sz;
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Reading size from %8x\n", app_start);
#endif /* ELF_LOAD_DEBUGGING */
		// need to align to a word boundry
		while ((app_start % 4) != 0)
		{
			app_start++;
		}
		sz = *(uint32_t*)app_start;
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Size = %x\n", sz);
#endif /* ELF_LOAD_DEBUGGING */
		app_start += 4;
	}
}
