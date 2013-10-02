/*
 *
 * SOS Source Code
 * __________________
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
#include "process/process_manager.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"
#include "kernel/utils/util_memset.h"
#include "kernel/objects/registry.h"
#include "arch/tgt.h"

/**
 * The kernel process
 */
static __process_t * __kernel_process = NULL;

/**
 * The kernel idle thread
 */
static __thread_t * __kernel_idle_thread = NULL;

void __kernel_initialise(void)
{
	const uint32_t memory_start = __bsp_get_usable_memory_start();
	const uint32_t memory_end = __bsp_get_usable_memory_end();

	__debug_print("Time: Initialising services...\n");
	__time_initialise();
	__alarm_initialse();

	__debug_print("Memory: Initialising Pool, start %X, end %x\n",
			memory_start,
			memory_end);

	__kernel_assert("Memory End > 0", memory_end > 0);
	__kernel_assert("Memory Size > 0", memory_end > memory_start);

	const bool_t mem_init_ok = __mem_initialise(
			memory_start,
			memory_end);

	__kernel_assert("Failed to Initialise Memory Manager", mem_init_ok);

	__debug_print("Process: Initialising Management...\n");

	__proc_initialise();

	__debug_print("Registry: Initialising the Registry...\n");

	__registry_initialise(__mem_get_default_pool());

	__debug_print("Kernel: Initialising Kernel Process...\n");

	__proc_create_process(
			"kernel",
			"kernel_idle",
			__kernel_idle,
			__KERNEL_IDLE_PRIORITY,
			__KERNEL_IDLE_STACK,
			__KERNEL_HEAP,
			THREAD_FLAG_NONE,
			&__kernel_process);

	__kernel_idle_thread = __process_get_main_thread(__kernel_process);

	__kernel_assert("Kernel Process not created",__kernel_process != NULL);
	__kernel_assert("Kernel Idle Thread not created", __kernel_idle_thread != NULL);

	__thread_set_state(__kernel_idle_thread, thread_system);

	__sch_initialise_scheduler();
}

__process_t * __kernel_get_process(void)
{
	return __kernel_process;
}

__thread_t * __kernel_get_idle_thread(void)
{
	return __kernel_idle_thread;
}
