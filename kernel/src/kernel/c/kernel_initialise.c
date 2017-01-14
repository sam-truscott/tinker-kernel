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

static void load_processes(const mem_t end_of_bin, loader_t* const loader)
{
	uint32_t* app_start = (uint32_t*)(end_of_bin + 4);
	debug_print(ELF_LOADER, "Loader: Loading first app at %8x\n", end_of_bin + (*app_start));
	while (*app_start)
	{
		const return_t elf_error = load_elf(loader, (void*) (end_of_bin + *app_start), "app", 128, 0);
		debug_print(ELF_LOADER, "Loader: Loading result: %d\n", elf_error);
		app_start++;
		debug_print(ELF_LOADER, "Loader: Loading next app at %8x\n", end_of_bin + *app_start);
	}
}

static mem_t calculate_start_of_pool(const mem_t end_of_bin)
{
	const mem_t size = *(mem_t*)end_of_bin;
	debug_print(INITIALISATION, "Size of payload is 0%8x\n", size);
	return end_of_bin + size;
}

void kernel_initialise(void)
{
	const mem_t end_of_bin = bsp_get_usable_memory_start();
	mem_t memory_start = calculate_start_of_pool(end_of_bin);
	const mem_t memory_end = bsp_get_usable_memory_end() - (MMU_PAGE_SIZE);

	debug_print(INITIALISATION, "Loader: Apps start at %x, mem at %x. Size of %x\n", end_of_bin, memory_start, memory_start - end_of_bin);

	mem_t aligned_start = memory_start;
	while ((aligned_start % MMU_PAGE_SIZE) != 0)
	{
		aligned_start++;
	}

	debug_print(INITIALISATION, "Memory: Initialising Pool, start %X (%x), end %x\n", memory_start, aligned_start, memory_end);

	kernel_assert("Memory End > 0", memory_end > 0);
	kernel_assert("Memory Size > 0", memory_end > aligned_start);

	const bool_t mem_init_ok = mem_initialise(aligned_start, memory_end);
	kernel_assert("Failed to Initialise Memory Manager", mem_init_ok);

	mem_pool_info_t * const pool = mem_get_default_pool();

	debug_prints(INITIALISATION, "Time: Initialising services...\n");
	time_manager_t * const time_manager = time_initialise(pool);
	alarm_manager_t * const alarm_manager = alarm_initialse(pool, time_manager);
	debug_print(INITIALISATION, "Time: Time Manager at %x, Alarm Manager at %x\n", time_manager, alarm_manager);

	debug_prints(INITIALISATION, "Registry: Initialising the Registry...\n");
	registry_t * const registry = registry_create(pool);
	debug_print(INITIALISATION, "Registry: Registry at %x\n", registry);

	debug_prints(INITIALISATION, "Scheduler: Initialising Scheduler...\n");
	scheduler_t * const scheduler = sch_create_scheduler(pool);
	debug_print(INITIALISATION, "Scheduler: Scheduler at %x\n", scheduler);

	debug_prints(INITIALISATION, "Process: Initialising Management...\n");
	proc_list_t * const proc_list = proc_create(pool, scheduler, alarm_manager);
	debug_print(INITIALISATION, "Process: Process list at %x\n", proc_list);

	debug_prints(INITIALISATION, "Loader: Initialising Loader...\n");
	loader_t * const loader = loader_create(pool, proc_list);

	debug_prints(INITIALISATION, "Syscall: Initialising...\n");
	syscall_handler_t * const syscall_handler = create_handler(
			pool,
			proc_list,
			registry,
			scheduler,
			time_manager,
			alarm_manager,
			loader);
	debug_print(INITIALISATION, "Syscall: Syscall Handler at %x\n", syscall_handler);

	debug_prints(INITIALISATION, "Intc: Initialising Interrupt Controller...\n");
	interrupt_controller_t * const interrupt_controller = int_create(pool, syscall_handler, scheduler);
	debug_print(INITIALISATION, "Intc: Initialising Interrupt Controller at %x\n", interrupt_controller);

	debug_prints(INITIALISATION, "Kernel: Initialising Kernel Process...\n");

	extern char * __text;
	extern char * __text_end;
	extern char * __data_end;
	char * text_pos = (char*)&__text;
	char * text_epos = (char*)&__text_end;
	char * data_pos = 0;
	char * data_end = (char*)&__data_end;
	tinker_mempart_t data =
	{
		/* data */
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
		/* code */
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
		/* wrapper */
		.heap_size = KERNEL_HEAP,
		.stack_size = KERNEL_IDLE_STACK,
		.first_part = &code
	};

	debug_print(INITIALISATION, "Kernel: Process list %x\n", proc_list);
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
	debug_prints(INITIALISATION, "BSP: Setting up the Board...\n");
	bsp_setup(
			interrupt_controller,
			time_manager,
			alarm_manager,
			kernel_process);
	debug_prints(INITIALISATION, "BSP: Setup Complete\n");

#if defined (KERNEL_SHELL)
	debug_prints(INITIALISATION, "System: Creating kshell\n");
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
