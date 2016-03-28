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
#if defined(KERNEL_INIT)
#include "console/print_out.h"
#endif /* KERNEL_INIT */
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

static void load_processes(const uint32_t end_of_bin, loader_t* const loader)
{
	uint32_t app_start = end_of_bin;
	uint32_t sz = *(uint32_t*) app_start;
#if defined(ELF_LOAD_DEBUGGING)
	debug_print("Size of %x is %x bytes\n", app_start, sz);
#endif /* ELF_LOAD_DEBUGGING */
	app_start += sizeof(uint32_t);
	while (sz)
	{
		const error_t elf_error = load_elf(loader, (void*) app_start, "app", 128, 0);
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Loading of process: %d\n", elf_error);
#else
		(void)elf_error;
#endif /* ELF_LOAD_DEBUGGING */
		app_start += sz;
		// need to align to a word boundry
		while ((app_start % sizeof(uint32_t)) != 0)
		{
			app_start++;
		}
		sz = *(uint32_t*) app_start;
#if defined(ELF_LOAD_DEBUGGING)
		debug_print("Size of %x is %x bytes\n", app_start, sz);
#endif /* ELF_LOAD_DEBUGGING */
		app_start += sizeof(uint32_t);
	}
}

static uint32_t calculate_start_of_pool(const uint32_t end_of_bin)
{
	uint32_t app_start = end_of_bin;
	uint32_t sz = *(uint32_t*) app_start;
	app_start += sizeof(uint32_t);
	while (sz)
	{
		app_start += sz;
		while ((app_start % sizeof(uint32_t)) != 0)
		{
			app_start++;
		}
		sz = *(uint32_t*) app_start;
		app_start += sizeof(uint32_t);
	}
	return app_start;
}

void kernel_initialise(void)
{
	const uint32_t end_of_bin = bsp_get_usable_memory_start();
	uint32_t memory_start = calculate_start_of_pool(end_of_bin);
	while (memory_start % MMU_PAGE_SIZE != 0)
	{
		memory_start++;
	}
	const uint32_t memory_end = bsp_get_usable_memory_end();

#if defined(KERNEL_INIT)
	const uint32_t size_of_apps = memory_start - end_of_bin;
	debug_print("Apps start at %x, size of %x\n", end_of_bin, size_of_apps);

	debug_print("Memory: Initialising Pool, start %X, end %x\n",
			memory_start,
			memory_end);
#endif /* KERNEL_INIT */

	kernel_assert("Memory End > 0", memory_end > 0);
	kernel_assert("Memory Size > 0", memory_end > memory_start);

	const bool_t mem_init_ok = mem_initialise(
			memory_start,
			memory_end);
	kernel_assert("Failed to Initialise Memory Manager", mem_init_ok);

	mem_pool_info_t * const pool = mem_get_default_pool();

#if defined(KERNEL_INIT)
	debug_print("Time: Initialising services...\n");
#endif /* KERNEL_INIT */
	time_manager_t * const time_manager = time_initialise(pool);
	alarm_manager_t * const alarm_manager = alarm_initialse(pool, time_manager);
#if defined(KERNEL_INIT)
	debug_print("Time: Time Manager at %x, Alarm Manager at %x\n", time_manager, alarm_manager);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("Registry: Initialising the Registry...\n");
#endif /* KERNEL_INIT */
	registry_t * const registry = registry_create(pool);
#if defined(KERNEL_INIT)
	debug_print("Registry: Registry at %x\n", registry);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("Scheduler: Initialising Scheduler...\n");
#endif /* KERNEL_INIT */
	scheduler_t * const scheduler = sch_create_scheduler(pool);
#if defined(KERNEL_INIT)
	debug_print("Scheduler: Scheduler at %x\n", scheduler);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("Process: Initialising Management...\n");
#endif /* KERNEL_INIT */
	proc_list_t * const proc_list = proc_create(pool, scheduler, alarm_manager);
#if defined(KERNEL_INIT)
	debug_print("Process: Process list at %x\n", proc_list);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("Loader: Initialising Loader...\n");
#endif /* KERNEL_INIT */
	loader_t * const loader = loader_create(pool, proc_list);

#if defined(KERNEL_INIT)
	debug_print("Syscall: Initialising...\n");
#endif /* KERNEL_INIT */
	syscall_handler_t * const syscall_handler = create_handler(
			pool,
			proc_list,
			registry,
			scheduler,
			time_manager,
			alarm_manager,
			loader);
#if defined(KERNEL_INIT)
	debug_print("Syscall: Syscall Handler at %x\n", syscall_handler);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("Intc: Initialising Interrupt Controller...\n");
#endif /* KERNEL_INIT */
	interrupt_controller_t * const interrupt_controller = int_create(pool, syscall_handler, scheduler);
#if defined(KERNEL_INIT)
	debug_print("Intc: Initialising Interrupt Controller at %x\n", interrupt_controller);
#endif /* KERNEL_INIT */

#if defined(KERNEL_INIT)
	debug_print("Kernel: Initialising Kernel Process...\n");
#endif /* KERNEL_INIT */

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

#if defined(KERNEL_INIT)
	debug_print("Kernel: Process list %x\n", proc_list);
#endif /* KERNEL_INIT */
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
			MMU_PAGE_SIZE,
			0,
			bsp_get_usable_memory_end(),
			MMU_RANDOM_ACCESS_MEMORY,
			MMU_KERNEL_ACCESS,
			MMU_READ_WRITE);
	tgt_map_memory(kernel_process, kernel_ram_sec);

	kernel_assert("Kernel couldn't start Idle Thread", kernel_idle_thread != NULL);
	sch_set_current_thread(scheduler, kernel_idle_thread);

	/* Get the BSP to configure itself */
#if defined(KERNEL_INIT)
	debug_print("BSP: Setting up the Board...\n");
#endif /* KERNEL_INIT */
	bsp_setup(interrupt_controller, time_manager, alarm_manager, kernel_process);
#if defined(KERNEL_INIT)
	debug_print("BSP: Setup Complete\n");
#endif /* KERNEL_INIT */

#if defined (KERNEL_SHELL)
#if defined (KERNEL_INIT)
	debug_print("System: Creating kshell\n");
#endif /* KERNEL_INIT */
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

	load_processes(end_of_bin, loader);
}
