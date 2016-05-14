/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "process.h"
#include "objects/object.h"
#include "utils/collections/unbounded_list.h"
#include "utils/collections/unbounded_list_iterator.h"

typedef struct proc_list_t proc_list_t;

UNBOUNDED_LIST_TYPE(process_list_t)
UNBOUNDED_LIST_ITERATOR_TYPE(process_list_it_t)
UNBOUNDED_LIST_ITERATOR_SPEC(extern, process_list_it_t, process_list_t, process_t*)

proc_list_t * proc_create(
		mem_pool_info_t * const pool,
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager);

void proc_set_kernel_process(
		proc_list_t * const list,
		process_t * const kernel_process);

process_t * proc_get_kernel_process(proc_list_t * const list);

return_t proc_create_process(
		proc_list_t * const list,
		const char * image,
		const char * initial_task_name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		tinker_meminfo_t * const meminfo,
		const uint32_t flags,
		process_t ** process);

return_t proc_create_thread(
		process_t * process,
		const char * const name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t flags,
		object_t ** thread_object,
		thread_t ** new_thread);

process_list_it_t * proc_list_procs(proc_list_t * const list);

void proc_delete_proc(
		proc_list_t * const list,
		process_t * const process);

#endif /* PROCESS_MANAGER_H_ */
