/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_TABLE_H_
#define OBJECT_TABLE_H_

#include "../kernel_types.h"
#include "../utils/collections/hashed_map.h"

error_t __obj_initialse_table(
		const __process_t * const proc,
		__object_table_t * const t);

error_t __obj_allocate_next_free_object(
		__mem_pool_info_t * const pool,
		__object_table_t * const t,
		__object_t ** o);

__object_t * __obj_get_thread_object(__object_table_t * t, uint32_t oid);

#endif /* OBJECT_TABLE_H_ */
