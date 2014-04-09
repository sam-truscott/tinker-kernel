/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "process.h"
#include "kernel/objects/object.h"
#include "kernel/utils/collections/unbounded_list.h"
#include "kernel/utils/collections/unbounded_list_iterator.h"

UNBOUNDED_LIST_TYPE(__process_list_t)
UNBOUNDED_LIST_ITERATOR_TYPE(__process_list_it_t)
UNBOUNDED_LIST_ITERATOR_SPEC(extern, __process_list_it_t, __process_list_t, __process_t*)
/*
 * Process Manager
 */

void __proc_initialise(void);

error_t __proc_create_process(
		const char * image,
		const char * initial_task_name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const sos_meminfo_t * const meminfo,
		const uint32_t flags,
		__process_t ** process);

error_t __proc_create_thread(
		__process_t * process,
		const char * const name,
		thread_entry_point * entry_point,
		const uint8_t priority,
		const uint32_t stack,
		const uint32_t flags,
		__object_t ** thread_object,
		__thread_t ** new_thread);

__process_list_it_t * __proc_list_procs(void);

void __proc_delete_proc(const __process_t * const process);

#endif /* PROCESS_MANAGER_H_ */
