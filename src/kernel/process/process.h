/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PROCESS_H_
#define PROCESS_H_

#include "kernel/process/thread.h"
#include "kernel/objects/object_table.h"
#include "kernel/utils/collections/unbounded_list.h"
#include "kernel/utils/collections/unbounded_list_iterator.h"

typedef struct __process_t __process_t;

UNBOUNDED_LIST_TYPE(thread_list_t)
UNBOUNDED_LIST_ITERATOR_TYPE(thread_list_it_t)
UNBOUNDED_LIST_ITERATOR_SPEC(extern, thread_list_it_t, thread_list_t, __thread_t*)

error_t __process_create(
		__mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool is_kernel,
		const __mem_pool_info_t * pool,
		__process_t ** process);

uint32_t __process_get_pid(const __process_t * const process);

bool __process_is_kernel(const __process_t * const process);

object_number_t __process_get_oid(const __process_t * const process);

void __process_set_oid(
		__process_t * const process,
		const object_number_t oid);

__mem_pool_info_t * __process_get_mem_pool(const __process_t * const process);

__object_table_t * __process_get_object_table(const __process_t * const process);

bool __process_add_thread(
		__process_t * const process,
		__thread_t * const thread,
		object_number_t * const objno);

const segment_info_t * __process_get_segment_info(const __process_t * const process);

void __process_set_segment_info(
		__process_t * const process,
		const segment_info_t * const seg);

__thread_t * _process_get_main_thread(const __process_t * process);

thread_list_it_t * __process_get_threads(const __process_t * const process);

mmu_section_t * __process_get_first_section(const __process_t * const process);

#endif /* PROCESS_H_ */
