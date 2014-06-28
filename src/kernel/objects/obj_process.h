/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_PROCESS_H_
#define OBJ_PROCESS_H_

#include "arch/tgt_types.h"
#include "kernel/process/process.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"

typedef struct object_process_t object_process_t;

object_process_t * obj_cast_process(object_t * o);

object_number_t obj_process_get_oid
	(const object_process_t * const o);

error_t obj_create_process(
		mem_pool_info_t * const pool,
		object_table_t * const table,
		const uint32_t process_id,
		process_t * const process,
		object_t ** object);

error_t obj_process_thread_exit(
		object_process_t * const o,
		object_thread_t * const thread);

void obj_process_exit(object_process_t * const o);

void obj_delete_process(object_process_t * const o);

uint32_t obj_process_pid(const object_process_t * const o);

#endif /* OBJ_PROCESS_H_ */
