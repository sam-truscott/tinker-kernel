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

#include "kernel/memory/mem_pool.h"
#include "kernel/objects/object.h"
#include "kernel/utils/collections/hashed_map.h"

typedef struct __object_table_t __object_table_t;

__object_table_t * __obj_table_create(__mem_pool_info_t * const pool);

error_t __obj_initialse_table(
		__object_table_t * const table,
		__mem_pool_info_t * const pool);

error_t __obj_add_object(
		__object_table_t * const t,
		__object_t * const obj,
		object_number_t * const objno);

__object_t * __obj_get_object(__object_table_t * t, object_number_t oid);

#endif /* OBJECT_TABLE_H_ */
