/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_malloc.h"

#include "kernel/memory/memory_manager.h"
#include "kernel/scheduler/scheduler.h"

void * __util_malloc(const uint32_t size)
{
	__thread_t * current_thread = __sch_get_current_thread();
	__process_t * current_process = current_thread->parent;
	return __mem_alloc(current_process->memory_pool, size);
}
