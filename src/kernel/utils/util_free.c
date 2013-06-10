/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_free.h"

#include "../memory/memory_manager.h"
#include "../scheduler/scheduler.h"

void __util_free(void * ptr)
{
	__thread_t * current_thread = __sch_get_current_thread();
	__process_t * current_process = current_thread->parent;
	__mem_free(current_process->memory_pool, ptr);
}
