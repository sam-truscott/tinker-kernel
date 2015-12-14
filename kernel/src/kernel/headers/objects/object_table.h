/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_TABLE_H_
#define OBJECT_TABLE_H_

#include "tinker_api_errors.h"
#include "memory/mem_pool.h"
#include "objects/object.h"
#include "utils/collections/hashed_map.h"
#include "utils/collections/hashed_map_iterator.h"

HASH_MAP_TYPE_T(object_map_t)
HASH_MAP_TYPE_ITERATOR_TYPE(object_table_it_t)
HASH_MAP_TYPE_ITERATOR_SPEC(extern, object_table_it_t, object_t*)

typedef struct object_table_t object_table_t;

object_table_t * obj_table_create(mem_pool_info_t * const pool);

void obj_table_delete(const object_table_t * const table);

error_t obj_initialse_table(
		object_table_t * const table,
		mem_pool_info_t * const pool);

error_t obj_add_object(
		object_table_t * const t,
		object_t * const obj,
		object_number_t * const objno);

error_t obj_remove_object(
		object_table_t * const t,
		const object_number_t objno);

object_t * obj_get_object(const object_table_t * const t, object_number_t oid);

object_table_it_t * obj_iterator(const object_table_t * t);

#endif /* OBJECT_TABLE_H_ */
