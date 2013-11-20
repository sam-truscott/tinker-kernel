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

#include "arch/tgt_types.h"
#include "kernel/process/thread.h"
#include "kernel/memory/mem_section.h"
#include "kernel/objects/object_table.h"

typedef struct __process_t __process_t;

HASH_MAP_TYPE_ITERATOR_TYPE(thread_it_t)
HASH_MAP_TYPE_ITERATOR_SPEC(extern, thread_it_t, __thread_t*)

error_t __process_create(
		__mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool_t is_kernel,
		const __mem_pool_info_t * pool,
		__process_t ** process);

uint32_t __process_get_pid(const __process_t * const process);

const char * __process_get_image(const __process_t * const process);

bool_t __process_is_kernel(const __process_t * const process);

object_number_t __process_get_oid(const __process_t * const process);

void __process_set_oid(
		__process_t * const process,
		const object_number_t oid);

__mem_pool_info_t * __process_get_mem_pool(const __process_t * const process);

__object_table_t * __process_get_object_table(const __process_t * const process);

bool_t __process_add_thread(
		__process_t * const process,
		__thread_t * const thread,
		object_number_t * const objno);

const tgt_mem_t * __process_get_segment_info(const __process_t * const process);

void __process_set_segment_info(
		__process_t * const process,
		const tgt_mem_t * const seg);

__thread_t * __process_get_main_thread(const __process_t * process);

uint32_t __process_get_thread_count(const __process_t * process);

const mem_section_t * __process_get_first_section(const __process_t * const process);

void __process_thread_exit(__process_t * const process, __thread_t * const thread);

void __process_exit(__process_t * const process);

uint32_t __process_virt_to_real(
		const __process_t * const process,
		const uint32_t virt);

uint32_t __process_find_free_vmem(
		const __process_t * const process,
		const uint32_t real_address,
		const uint32_t size,
		const mmu_memory_t type,
		const mmu_privilege_t priv,
		const mmu_access_t access);

thread_it_t * __process_iterator(const __process_t * const process);

#endif /* PROCESS_H_ */
