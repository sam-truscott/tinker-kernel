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

HASH_MAP_INTERNAL_TYPE_T(object_map_t, object_number_t, object_t*, MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_SPEC_CREATE(static, object_map_t)
HASH_MAP_SPEC_INITALISE(static, object_map_t)
HASH_MAP_SPEC_DELETE(static, object_map_t)
HASH_MAP_SPEC_CAPACITY(static, object_map_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, object_map_t, object_number_t)
HASH_MAP_SPEC_PUT(static, object_map_t, object_number_t, object_t*)
HASH_MAP_SPEC_GET(static, object_map_t, object_number_t, object_t*)
HASH_MAP_SPEC_REMOVE(static, object_map_t, object_number_t, object_t*)
HASH_FUNCS_VALUE(object_map_t, object_number_t)
HASH_MAP_BODY_CREATE(static, object_map_t)
HASH_MAP_BODY_INITALISE(static, object_map_t, MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_BODY_DELETE(static, object_map_t, MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_BODY_CAPACITY(static, object_map_t, MAX_OBJECT_TABLE_SIZE)
HASH_MAP_BODY_CONTAINS_KEY(static, object_map_t, object_number_t, 16)
HASH_MAP_BODY_PUT(static, object_map_t, object_number_t, object_t*, MAX_OBJECT_TABLE_SIZE, 16)
HASH_MAP_BODY_GET(static, object_map_t, object_number_t, object_t*, 16)
HASH_MAP_BODY_REMOVE(static, object_map_t, object_number_t, 16)

HASH_MAP_TYPE_ITERATOR_INTERNAL_TYPE(object_table_it_t, object_map_t)
HASH_MAP_TYPE_ITERATOR_BODY(extern, object_table_it_t, object_map_t, object_number_t, object_t*, MAX_OBJECT_TABLE_SIZE, 16)

typedef struct object_table_t
{
	mem_pool_info_t * pool;
	object_map_t * the_map;
	object_number_t next_id;
} object_table_internal_t;

object_table_t * obj_table_create(mem_pool_info_t * const pool)
{
	object_table_t * table = mem_alloc(pool, sizeof(object_table_t));
	if (table)
	{
		obj_initialse_table(table, pool);
	}
	return table;
}

void obj_table_delete(const object_table_t * const table)
{
	kernel_assert("obj_table_delete - check that the object table is present", table != NULL);
	object_map_t_delete(table->the_map);
	mem_free(table->pool, table);
}

static bool_t hash_equal_object_number(const object_number_t l, const object_number_t r)
{
	return (l == r);
}

error_t obj_initialse_table(
		object_table_t * const table,
		mem_pool_info_t * const pool)
{
	error_t ret = NO_ERROR;

	/* create the hashmap for the object table */
	if (table)
	{
		table->pool = pool;
		table->the_map = object_map_t_create(
				hash_basic_integer,
				hash_equal_object_number,
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

error_t obj_add_object(
		object_table_t * const t,
		object_t * const obj,
		object_number_t * const objno)
{
	bool_t id_ok = false;
	error_t ret = NO_ERROR;

	if (t && obj)
	{
		/* find the next suitable object id to use */
		object_number_t id = t->next_id;
		const uint32_t tc = object_map_t_capacity(t->the_map);
		for ( ; id != tc ; id++ )
		{
			if ( !object_map_t_contains_key(t->the_map, id) )
			{
				id_ok = true;
				break;
			}
		}

		/* now create an allocate the object */
		if (id_ok)
		{
			if ( !object_map_t_put((t->the_map), id, obj))
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

error_t obj_remove_object(
		object_table_t * const t,
		const object_number_t objno)
{
	error_t ret = NO_ERROR;

	if (t)
	{
		if (object_map_t_contains_key(t->the_map, objno))
		{
			if(!object_map_t_remove(t->the_map, objno))
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

object_t * obj_get_object(const object_table_t * const t, object_number_t oid)
{
	object_t * o = NULL;
	if (t)
	{
		object_t * tmp_object = NULL;
		if (object_map_t_get(t->the_map, oid, &tmp_object))
		{
			o = tmp_object;
		}
	}
	return o;
}

object_table_it_t * obj_iterator(const object_table_t * t)
{
	return object_table_it_t_create(t->the_map);
}
