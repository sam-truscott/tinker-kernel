/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "object_table.h"

#include "config.h"
#include "object.h"
#include "obj_thread.h"
#include "kernel/process/process.h"
#include "kernel/utils/util_malloc.h"
#include "kernel/memory/memory_manager.h"

HASH_MAP_TYPE_T(object_map_t, object_number_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)
HASH_MAP_SPEC_T(static, object_map_t, object_number_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)
HASH_MAP_BODY_T(static, object_map_t, object_number_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)

typedef struct __object_table_t
{
	object_map_t * the_map;
	object_number_t next_id;
} __object_table_internal_t;

__object_table_t * __obj_table_create(__mem_pool_info_t * const pool)
{
	__object_table_t * table = __mem_alloc(pool, sizeof(__object_table_t));
	if (table)
	{
		__obj_initialse_table(table, pool);
	}
	return table;
}

error_t __obj_initialse_table(
		__object_table_t * const table,
		__mem_pool_info_t * const pool)
{
	error_t ret = NO_ERROR;

	/* create the hashmap for the object table */
	if ( table )
	{
		table->the_map = __mem_alloc(pool, sizeof(object_map_t));
		table->the_map = object_map_t_create(
				__hash_basic_integer,
				pool);
		if  (table->the_map )
		{
			table->next_id = 1;
		}
		else
		{
			ret = OUT_OF_MEMORY;
		}
	}
	return ret;
}

error_t __obj_add_object(
		__object_table_t * const t,
		__object_t * const obj,
		object_number_t * const objno)
{
	bool id_ok = false;
	error_t ret = NO_ERROR;

	if ( t && obj)
	{
		object_map_t * map = (object_map_t*)t->the_map;
		/* find the next suitable object id to use */
		object_number_t id = t->next_id;
		const uint32_t tc = object_map_t_capacity(map);
		for ( ; id != tc ; id++ )
		{
			if ( !object_map_t_contains_key(map, id) )
			{
				id_ok = true;
				break;
			}
		}

		/* now create an allocate the object */
		if ( id_ok )
		{
			t->next_id++;
			if ( !object_map_t_put(((object_map_t*)t->the_map), id, obj))
			{
				ret = OBJECT_ADD_FAILED;
			}
			*objno = id;
		} else {
			ret = OBJECT_TABLE_FULL;
		}
	} else {
		ret = PARAMETERS_INVALID;
	}

	return ret;
}

__object_t * __obj_get_object(const __object_table_t * t, object_number_t oid)
{
	__object_t * o = NULL;
	if ( t )
	{
		__object_t * tmp_object = NULL;
		object_map_t * map = (object_map_t*)t->the_map;

		if ( object_map_t_get(map, oid, &tmp_object) )
		{
			o = tmp_object;
		}
	}
	return o;
}
