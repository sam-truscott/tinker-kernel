/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */


#include "process/test_proc_list.h"

#pragma GCC diagnostic ignored "-Wunused-function"

#include "kernel_assert.h"
#include "process/process_list.h"
#include "process/process_list_private.h"
#include "scheduler/scheduler.h"
#include "scheduler/scheduler_private.h"
#include "process/process_private.h"
#include "process/thread_private.h"
#include "utils/util_memset.h"

#define SIZE (1024 * 128)

void test_proc_list(void)
{
	char * pool_mem = (char*)mem_alloc(mem_get_default_pool(), SIZE);
	mem_pool_info_t * pool = NULL;
	mem_init_memory_pool(
			(mem_t)pool_mem,
			SIZE,
			&pool);

	scheduler_t * const sch = sch_create_scheduler(pool);
	// TODO mock alarm manager
	// TODO mock kernel process
	process_t kproc;
	util_memset(&kproc, 0, sizeof(kproc));
	kproc.kernel_process = true;
	kproc.process_id = 1;
	kproc.parent_pool = pool;
	kproc.private_pool = pool;

	proc_list_t * const list = proc_create(pool, sch, NULL);

	const mem_t allocated = mem_get_allocd_size(pool);

	kernel_assert("proc list should be non null", list != NULL);
	kernel_assert("last proc id should be one", list->last_pid == 1);
	kernel_assert("should be null", list->kernel_process == NULL);
	kernel_assert("should be non null", list->process_list != NULL);
	kernel_assert("should be zero", list_size(list->process_list) == 0);

	proc_set_kernel_process(list, &kproc);
	process_t * proc = proc_get_kernel_process(list);
	kernel_assert("should be equal", proc == (&kproc));

	thread_t idle;
	idle.parent = &kproc;
	sch->curr_thread = &idle;

	tinker_meminfo_t kp_mem_info;
	kp_mem_info.heap_size = 1024;
	kp_mem_info.stack_size = 1024;
	return_t err = proc_create_process(list, "proc", "task", NULL, 2, &kp_mem_info, 0, &proc);
	kernel_assert("should be non null", err == NO_ERROR);
	kernel_assert("should be non null", proc != NULL);
	kernel_assert("should be one", list_size(list->process_list) == 1);

	list_it_t * const it = proc_list_procs(list);
	process_t * item = NULL;
	kernel_assert("should be true", list_it_get(it, &item));
	kernel_assert("should be equal", item == proc);
	kernel_assert("should be false", !list_it_next(it, &item));
	list_it_delete(it);

	thread_t * const task = process_get_main_thread(proc);
	process_thread_exit(proc, task);
	process_exit(proc);
	proc_delete_proc(list, proc);

	kernel_assert("should be zero", list_size(list->process_list) == 0);

	kernel_assert("should be equal", allocated == mem_get_allocd_size(pool));
	mem_free(mem_get_default_pool(), pool_mem);
}


