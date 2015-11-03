/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_PROCESS_PROCESS_PRIVATE_H_
#define KERNEL_PROCESS_PROCESS_PRIVATE_H_

#include "config.h"
#include "arch/tgt.h"
#include "kernel/process/thread.h"
#include "kernel/utils/collections/hashed_map.h"

HASH_MAP_TYPE_T(thread_map_t)
HASH_MAP_INTERNAL_TYPE_T(thread_map_t, uint32_t, thread_t*, MAX_THREADS, 16)
HASH_MAP_SPEC_CREATE(static, thread_map_t)
HASH_MAP_SPEC_INITALISE(static, thread_map_t)
HASH_MAP_SPEC_SIZE(static, thread_map_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, thread_map_t, uint32_t)
HASH_MAP_SPEC_PUT(static, thread_map_t, uint32_t, thread_t*)
HASH_MAP_SPEC_REMOVE(static, thread_map_t, uint32_t, thread_t*)
HASH_MAP_SPEC_DELETE(static, thread_map_t)
HASH_FUNCS_VALUE(thread_map_t, uint32_t)
HASH_MAP_BODY_CREATE(static, thread_map_t)
HASH_MAP_BODY_INITALISE(static, thread_map_t, MAX_THREADS, 16)
HASH_MAP_BODY_SIZE(static, thread_map_t)
HASH_MAP_BODY_CONTAINS_KEY(static, thread_map_t, uint32_t, 16)
HASH_MAP_BODY_PUT(static, thread_map_t, uint32_t, thread_t*, MAX_THREADS, 16)
HASH_MAP_BODY_REMOVE(static, thread_map_t, uint32_t, 16)
HASH_MAP_BODY_DELETE(static, thread_map_t, MAX_THREADS, 16)

typedef struct process_t
{
	scheduler_t * 			scheduler;
	alarm_manager_t * 		alarm_manager;
	uint32_t				process_id;
	thread_map_t	*		threads;
	mem_pool_info_t * 		private_pool;
	mem_pool_info_t * 		memory_pool;
	object_table_t *		object_table;
	object_number_t			object_number;
	bool_t					kernel_process;
	tgt_mem_t				mem_info;
	mem_section_t *			first_section;
	char					image[MAX_PROCESS_IMAGE_LEN + 1];
	uint32_t 				next_thread_id;
	thread_t * 				initial_thread;
	mem_pool_info_t * 		parent_pool;
	tgt_pg_tbl_t *			page_table;
} process_internal_t;

#endif /* KERNEL_PROCESS_PROCESS_PRIVATE_H_ */
