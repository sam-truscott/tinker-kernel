/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#pragma GCC diagnostic ignored "-Wunused-function"

#include "scheduler/test_scheduler.h"
#include "kernel_assert.h"
#include "scheduler/scheduler.h"
#include "scheduler/scheduler_private.h"
#include "process/thread.h"
#include "process/thread_private.h"
#include "utils/util_memset.h"
#include "utils/util_memcpy.h"

#define SIZE (1024 * 8)

void test_scheduler(void)
{
	mem_pool_info_t * pool = NULL;
	char * pool_mem = (char*)mem_alloc(mem_get_default_pool(), SIZE);
	mem_init_memory_pool(
				(mem_t)pool_mem,
				SIZE,
				&pool);

	scheduler_t * const sch = sch_create_scheduler(pool);
	kernel_assert("scheduler should be non null", sch != NULL);
	kernel_assert("kernel idle thread should be null", sch->idle_thread == NULL);
	kernel_assert("kernel idle thread should be 0", sch->curr_priority == 0);
	kernel_assert("kernel idle thread should be null", sch->curr_thread == NULL);
	kernel_assert("kernel idle thread should be null", sch->curr_queue == sch->priority_queues[0]);

	thread_t idle_thread;
	util_memset(&idle_thread, 0, sizeof(idle_thread));
	idle_thread.context = NULL;
	idle_thread.entry_point = (thread_entry_point*)0x123;
	idle_thread.flags = 0;
	util_memcpy(idle_thread.name, "idle", 4);
	idle_thread.object_number = 56;
	idle_thread.parent = (process_t*)0x456;
	idle_thread.priority = 0;
	idle_thread.state = THREAD_SYSTEM;

	sch_set_kernel_idle_thread(sch, &idle_thread);
	sch_set_current_thread(sch, &idle_thread);
	kernel_assert("kernel idle thread should be 0x123", sch->idle_thread == &idle_thread);

	sch_notify_new_thread(sch, &idle_thread);
	kernel_assert("thread should be 1", sch->curr_priority == 0);
	kernel_assert("thread should be equal", sch->curr_queue == sch->priority_queues[0]);
	queue_t * q = NULL;
	stack_front(sch->queue_stack, (void**)&q);
	kernel_assert("thread should be equal", sch->curr_queue == q);

	sch_set_context_for_next_thread(sch, NULL, THREAD_READY);

	thread_t thread;
	util_memset(&thread, 0, sizeof(thread));
	thread.context = NULL;
	thread.entry_point = (thread_entry_point*)0x123;
	thread.flags = 0;
	util_memcpy(thread.name, "thread", 6);
	thread.object_number = 56;
	thread.parent = (process_t*)0x456;
	thread.priority = 1;
	thread.state = THREAD_READY;

	sch_notify_new_thread(sch, &thread);
	kernel_assert("thread should be 1", sch->curr_priority == 1);
	kernel_assert("thread should be equal", sch->curr_queue == sch->priority_queues[1]);
	q = NULL;
	stack_front(sch->queue_stack, (void**)&q);
	kernel_assert("thread should be equal", sch->curr_queue == q);

	sch_set_context_for_next_thread(sch, NULL, THREAD_READY);
	kernel_assert("thread should be equal", &thread == sch_get_current_thread(sch));

	thread.priority = 2;
	sch_notify_change_priority(sch, &thread, 1);
	kernel_assert("thread should be 2", sch->curr_priority == 2);
	kernel_assert("thread should be equal", sch->curr_queue == sch->priority_queues[2]);

	thread.priority = 1;
	sch_notify_change_priority(sch, &thread, 2);
	kernel_assert("thread should be 1", sch->curr_priority == 1);
	kernel_assert("thread should be equal", sch->curr_queue == sch->priority_queues[1]);

	sch_notify_exit_thread(sch, &thread);
	kernel_assert("thread should be 0", sch->curr_priority == 0);
	kernel_assert("thread should be equal", sch->curr_queue == sch->priority_queues[0]);
	q = NULL;
	stack_front(sch->queue_stack, (void**)&q);
	kernel_assert("thread should be equal", sch->curr_queue == q);

	sch_set_context_for_next_thread(sch, NULL, THREAD_READY);
	kernel_assert("thread should be equal", &idle_thread == sch_get_current_thread(sch));

	// TODO sch_terminate_current_thread

	mem_free(mem_get_default_pool(), pool_mem);
}
