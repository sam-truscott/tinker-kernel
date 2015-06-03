/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_initialise.h"

#include "../arch/board_support.h"
#include "kernel_assert.h"
#include "kernel_idle.h"
#include "console/print_out.h"
#include "interrupts/interrupt_manager.h"
#include "memory/memory_manager.h"
#include "process/process_list.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"
#include "kernel/utils/util_memset.h"
#include "kernel/objects/registry.h"
#include "kernel/syscall/syscall_handler.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/shell/kshell.h"
#include "arch/tgt.h"

static process_t * kernel_process = NULL;

static thread_t * kernel_idle_thread = NULL;

static proc_list_t * proc_list = NULL;

static syscall_handler_t * syscall_handler = NULL;

static interrupt_controller_t * interrupt_controller = NULL;

static registry_t * registry = NULL;

static scheduler_t * scheduler = NULL;

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
	time_initialise();
	alarm_initialse(pool);

	debug_print("Registry: Initialising the Registry...\n");
	registry = registry_create(pool);

	debug_print("Scheduler: Initialising Scheduler...\n");
	scheduler = sch_create_scheduler(pool);

	debug_print("Process: Initialising Management...\n");
	proc_list = proc_create(pool, scheduler);

	debug_print("Syscall: Initialising...\n");
	syscall_handler = create_handler(pool, proc_list, registry, scheduler);

	debug_print("Intc: Initialising Interrupt Controller...\n");
	interrupt_controller = int_create(pool, syscall_handler, scheduler);

	debug_print("Kernel: Initialising Kernel Process...\n");

	extern char * __text;
	extern char * __text_end;
	extern char * __data_end;
	char * text_pos = (char*)&__text;
	char * text_epos = (char*)&__text_end;
	char * data_pos = 0;
	char * data_end = (char*)&__data_end;
	const tinker_meminfo_t meminfo =
	{
		.heap_size = KERNEL_HEAP,
		.stack_size = KERNEL_IDLE_STACK,
		.text_start = (uint32_t)text_pos,
		.text_size = (uint32_t)(text_epos - text_pos),
		.data_start = (uint32_t)data_pos,
		.data_size = (uint32_t)(data_end - data_pos)
	};
	proc_create_process(
			proc_list,
			"kernel",
			"kernel_idle",
			kernel_idle,
			KERNEL_IDLE_PRIORITY,
			&meminfo,
			THREAD_FLAG_NONE,
			&kernel_process);

	kernel_idle_thread = process_get_main_thread(kernel_process);

	kernel_assert("Kernel Process not created",kernel_process != NULL);
	kernel_assert("Kernel Idle Thread not created", kernel_idle_thread != NULL);

	thread_set_state(kernel_idle_thread, THREAD_SYSTEM);

	kshell_setup(pool, scheduler, registry, proc_list);
}

scheduler_t * kernel_get_sch(void)
{
	return scheduler;
}

registry_t * kernel_get_reg(void)
{
	return registry;
}

interrupt_controller_t * kernel_get_intc(void)
{
	return interrupt_controller;
}

proc_list_t * kernel_get_proc_list(void)
{
	return proc_list;
}

process_t * kernel_get_process(void)
{
	return kernel_process;
}

thread_t * kernel_get_idle_thread(void)
{
	return kernel_idle_thread;
}
