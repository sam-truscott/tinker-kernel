/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_free.h"

#include "kernel/memory/memory_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process.h"

void __util_free(void * ptr)
{
	__thread_t * current_thread = __sch_get_current_thread();
	__process_t * const current_process = __thread_get_parent(current_thread);
	__mem_pool_info_t * const mempool = __process_get_mem_pool(current_process);
	__mem_free(mempool, ptr);
}
#pragma weak __util_free
