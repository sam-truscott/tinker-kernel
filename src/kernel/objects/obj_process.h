/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_PROCESS_H_
#define OBJ_PROCESS_H_

#include "arch/tgt_types.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"

typedef struct __object_process_t __object_process_t;

__object_process_t * __obj_cast_process(__object_t * o);

object_number_t __obj_process_get_oid
	(const __object_process_t * const o);

error_t __obj_create_process(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		const uint32_t process_id,
		__object_t ** object);

void __obj_delete_process(__object_process_t * const o);

#endif /* OBJ_PROCESS_H_ */
