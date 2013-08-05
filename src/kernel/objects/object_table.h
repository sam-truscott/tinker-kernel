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

#include "sos_api_errors.h"
#include "kernel/memory/mem_pool.h"
#include "kernel/objects/object.h"
#include "kernel/utils/collections/hashed_map.h"
#include "kernel/utils/collections/hashed_map_iterator.h"

HASH_MAP_TYPE_T(object_map_t)
HASH_MAP_TYPE_ITERATOR_TYPE(object_table_it_t)
HASH_MAP_TYPE_ITERATOR_SPEC(extern, object_table_it_t, object_map_t, __object_t*)

typedef struct __object_table_t __object_table_t;

__object_table_t * __obj_table_create(__mem_pool_info_t * const pool);

void __obj_table_delete(const __object_table_t * const table);

error_t __obj_initialse_table(
		__object_table_t * const table,
		__mem_pool_info_t * const pool);

error_t __obj_add_object(
		__object_table_t * const t,
		__object_t * const obj,
		object_number_t * const objno);

error_t __obj_remove_object(
		__object_table_t * const t,
		const object_number_t objno);

__object_t * __obj_get_object(const __object_table_t * t, object_number_t oid);

object_table_it_t * __obj_iterator(const __object_table_t * t);

#endif /* OBJECT_TABLE_H_ */
