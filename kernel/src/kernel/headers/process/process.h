/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PROCESS_H_
#define PROCESS_H_

#include "tgt_types.h"
#include "process/thread.h"
#include "memory/mem_section.h"
#include "objects/object_table.h"
#include "scheduler/scheduler.h"
#include "time/alarm_manager.h"

typedef struct process_t process_t;

HASH_MAP_TYPE_ITERATOR_TYPE(thread_it_t)
HASH_MAP_TYPE_ITERATOR_SPEC(extern, thread_it_t, thread_t*)

error_t process_create(
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager,
		mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool_t is_kernel,
		tinker_meminfo_t * const meminfo,
		mem_pool_info_t * pool,
		const mem_section_t * ksection,
		process_t ** process);

uint32_t process_get_pid(const process_t * const process);

const char * process_get_image(const process_t * const process);

bool_t process_is_kernel(const process_t * const process);

object_number_t process_get_oid(const process_t * const process);

void process_set_oid(
		process_t * const process,
		const object_number_t oid);

mem_pool_info_t * process_get_mem_pool(const process_t * const process);

mem_pool_info_t * process_get_user_mem_pool(const process_t * const process);

object_table_t * process_get_object_table(const process_t * const process);

bool_t process_add_thread(
		process_t * const process,
		thread_t * const thread,
		object_number_t * const objno);

const tgt_mem_t * process_get_mem_info(const process_t * const process);

void process_set_mem_info(
		process_t * const process,
		const tgt_mem_t * const seg);

thread_t * process_get_main_thread(const process_t * process);

uint32_t process_get_thread_count(const process_t * process);

const mem_section_t * process_get_first_section(const process_t * const process);

void process_thread_exit(process_t * const process, thread_t * const thread);

void process_exit(process_t * const process);

uint32_t process_virt_to_real(
		const process_t * const process,
		const uint32_t virt);

error_t process_allocate_vmem(
		process_t * const process,
		const uint32_t real_address,
		const uint32_t size,
		const mmu_memory_t type,
		const mmu_privilege_t priv,
		const mmu_access_t access,
		uint32_t * const virt_address);

void process_free_vmem(
		const process_t * const process,
		const uint32_t virt_address);

thread_it_t * process_iterator(const process_t * const process);

tgt_pg_tbl_t * process_get_page_table(const process_t * const process);

#endif /* PROCESS_H_ */
