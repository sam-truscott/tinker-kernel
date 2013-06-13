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
#include "kernel/scheduler/round_robin/scheduler_rr.h"
#include "kernel/scheduler/priority/scheduler_priority.h"
#include "alarms/alarm_manager.h"
#include "debug/debug_print.h"
#include "interrupts/interrupt_manager.h"
#include "memory/memory_manager.h"
#include "process/process_manager.h"
#include "time/time_manager.h"
#include "kernel/utils/util_memset.h"
#include "arch/tgt.h"

/**
 * The kernel process
 */
static __process_t * __kernel_process = NULL;

/**
 * The kernel idle thread
 */
static __thread_t * __kernel_idle_thread = NULL;

/**
 * The kernel scheduler thread
 */
static __thread_t * __kernel_sch_thread = NULL;

void __kernel_initialise(void)
{
	const uint32_t memory_start = __bsp_get_usable_memory_start();
	const uint32_t memory_end = __bsp_get_usable_memory_end();

	__debug_print("Interrupts: Initialising services...\n");
	__int_initialise();

	__debug_print("Time: Initialising services...\n");
	__time_initialise();
	__alarm_initialse();

	__debug_print("Memory: Initialising Pool, start %X, end %x\n",
			memory_start,
			memory_end);

	__kernel_assert("Memory End > 0", memory_end > 0);
	__kernel_assert("Memory Size > 0", memory_end > memory_start);

	const bool mem_init_ok = __mem_initialise(
			memory_start,
			memory_end);

	__kernel_assert("Failed to Initialise Memory Manager", mem_init_ok);

	__debug_print("Process: Initialising Management...\n");

	__proc_initialise();

	/* TODO we should call the rr scheduler to get this structure */
	__scheduler_t sch;
	__util_memset(&sch, 0, sizeof(sch));
#define SCH_PRIORITY
#if defined (SCH_ROUND_ROBIN)
	sch.initialise = __sc_rr_initialise;
	sch.scheduler = __sch_rr_execute;
	sch.get_curr_thread = __sch_rr_get_curr_thread;
	sch.set_curr_thread = __sch_rr_set_curr_thread;
#endif /* SCH_ROUND_ROBIN*/
#if defined (SCH_PRIORITY)
	sch.initialise = __sch_priority_initialise;
	sch.scheduler = __sch_priority_execute;
	sch.get_curr_thread = __sch_priority_get_curr_thread;
	sch.set_curr_thread = __sch_priority_set_curr_thread;
	sch.new_thread = __sch_priority_notify_new_thread;
	sch.exit_thread = __sch_priority_notify_exit_thread;
	sch.pause_thread = __sch_priority_notify_pause_thread;
	sch.resume_thread = __sch_priority_notify_resume_thread;
	sch.change_priority = __sch_priority_notify_change_priority;
#endif /* SCH_PRIORITY */
	__sch_initialise(&sch);

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

	__kernel_idle_thread = __proc_get_thread(__kernel_process, 0);

	__kernel_assert("Kernel Process not created",__kernel_process != NULL);
	__kernel_assert("Kernel Idle Thread not created", __kernel_idle_thread != NULL);

	__kernel_idle_thread->state = thread_system;

	__proc_create_thread(
			__kernel_idle_thread->parent,
			"scheduler",
			__sch_scheduler,
			0,
			0x1000,
			0,
			NULL,
			&__kernel_sch_thread);
	__tgt_disable_thread_interrupts(__kernel_sch_thread);

	__kernel_sch_thread->state = thread_system;
	__kernel_assert("Kernel couldn't start Scheduler Thread", __kernel_sch_thread != NULL);

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

__thread_t * __kernel_get_scheduler_thread(void)
{
	return __kernel_sch_thread;
}
