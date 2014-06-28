/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "object_table.h"

#include "config.h"
#include "object.h"
#include "obj_thread.h"
#include "kernel/kernel_assert.h"
#include "kernel/process/process.h"
#include "kernel/memory/memory_manager.h"

HASH_MAP_INTERNAL_TYPE_T(__object_map_t, object_number_t, __object_t*, __MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_SPEC_CREATE(static, __object_map_t)
HASH_MAP_SPEC_INITALISE(static, __object_map_t)
HASH_MAP_SPEC_DELETE(static, __object_map_t)
HASH_MAP_SPEC_CAPACITY(static, __object_map_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, __object_map_t, object_number_t)
HASH_MAP_SPEC_PUT(static, __object_map_t, object_number_t, __object_t*)
HASH_MAP_SPEC_GET(static, __object_map_t, object_number_t, __object_t*)
HASH_MAP_SPEC_REMOVE(static, __object_map_t, object_number_t, __object_t*)
HASH_FUNCS_VALUE(__object_map_t, object_number_t)
HASH_MAP_BODY_CREATE(static, __object_map_t)
HASH_MAP_BODY_INITALISE(static, __object_map_t, __MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_BODY_DELETE(static, __object_map_t, __MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_BODY_CAPACITY(static, __object_map_t, __MAX_OBJECT_TABLE_SIZE)
HASH_MAP_BODY_CONTAINS_KEY(static, __object_map_t, object_number_t, 16)
HASH_MAP_BODY_PUT(static, __object_map_t, object_number_t, __object_t*, __MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_BODY_GET(static, __object_map_t, object_number_t, __object_t*, 16)
HASH_MAP_BODY_REMOVE(static, __object_map_t, object_number_t, 16)

HASH_MAP_TYPE_ITERATOR_INTERNAL_TYPE(__object_table_it_t, __object_map_t)
HASH_MAP_TYPE_ITERATOR_BODY(extern, __object_table_it_t, __object_map_t, object_number_t, __object_t*, __MAX_OBJECT_TABLE_SIZE, 16)

typedef struct __object_table_t
{
	__mem_pool_info_t * pool;
	__object_map_t * the_map;
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

void __obj_table_delete(const __object_table_t * const table)
{
	__kernel_assert("__obj_table_delete - check that the object table is present", table != NULL);
	__object_map_t_delete(table->the_map);
	__mem_free(table->pool, table);
}

static bool_t __hash_equal_object_number(const object_number_t l, const object_number_t r)
{
	return (l == r);
}

error_t __obj_initialse_table(
		__object_table_t * const table,
		__mem_pool_info_t * const pool)
{
	error_t ret = NO_ERROR;

	/* create the hashmap for the object table */
	if (table)
	{
		table->pool = pool;
		table->the_map = __object_map_t_create(
				__hash_basic_integer,
				__hash_equal_object_number,
				true,
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
	bool_t id_ok = false;
	error_t ret = NO_ERROR;

	if (t && obj)
	{
		/* find the next suitable object id to use */
		object_number_t id = t->next_id;
		const uint32_t tc = __object_map_t_capacity(t->the_map);
		for ( ; id != tc ; id++ )
		{
			if ( !__object_map_t_contains_key(t->the_map, id) )
			{
				id_ok = true;
				break;
			}
		}

		/* now create an allocate the object */
		if (id_ok)
		{
			if ( !__object_map_t_put((t->the_map), id, obj))
			{
				ret = OBJECT_ADD_FAILED;
			}
			else
			{
				t->next_id = id + 1;
				*objno = id;
			}
		}
		else
		{
			ret = OBJECT_TABLE_FULL;
		}
	}
	else
	{
		ret = PARAMETERS_INVALID;
	}

	return ret;
}

error_t __obj_remove_object(
		__object_table_t * const t,
		const object_number_t objno)
{
	error_t ret = NO_ERROR;

	if (t)
	{
		if (__object_map_t_contains_key(t->the_map, objno))
		{
			if(!__object_map_t_remove(t->the_map, objno))
			{
				ret = UNKNOWN_OBJ;
			}
		}
		else
		{
			ret = INVALID_OBJECT;
		}
	}

	return ret;
}

__object_t * __obj_get_object(const __object_table_t * t, object_number_t oid)
{
	__object_t * o = NULL;
	if (t)
	{
		__object_t * tmp_object = NULL;
		if (__object_map_t_get(t->the_map, oid, &tmp_object))
		{
			o = tmp_object;
		}
	}
	return o;
}

__object_table_it_t * __obj_iterator(const __object_table_t * t)
{
	return __object_table_it_t_create(t->the_map);
}
