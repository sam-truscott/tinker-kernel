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
#include "tgt.h"
#include "process/thread.h"
#include "utils/collections/hashed_map.h"

typedef struct process_t
{
	scheduler_t * 			scheduler;
	alarm_manager_t * 		alarm_manager;
	uint32_t				process_id;
	thread_t *				threads[MAX_THREADS];
	mem_pool_info_t * 		private_pool;
	mem_pool_info_t * 		memory_pool;
	object_table_t *		object_table;
	object_number_t			object_number;
	bool_t					kernel_process;
	tgt_mem_t				mem_info;
	mem_section_t *			first_section;
	char					image[MAX_PROCESS_IMAGE_LEN + 1];
	thread_t * 				initial_thread;
	mem_pool_info_t * 		parent_pool;
	tgt_pg_tbl_t *			page_table;
} process_internal_t;

#endif /* KERNEL_PROCESS_PROCESS_PRIVATE_H_ */
