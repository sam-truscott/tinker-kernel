/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
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
