/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/object_table.h"

#include "config.h"
#include "objects/object.h"
#include "objects/obj_thread.h"
#include "kernel_assert.h"
#include "process/process.h"
#include "memory/memory_manager.h"
#include "utils/util_memcpy.h"
#include "utils/hash/basic_hashes.h"

typedef struct object_table_t
{
	mem_pool_info_t * pool;
	map_t * the_map;
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
	map_delete(table->the_map);
	mem_free(table->pool, table);
}

return_t obj_initialse_table(
		object_table_t * const table,
		mem_pool_info_t * const pool)
{
	return_t ret = NO_ERROR;

	/* create the hashmap for the object table */
	if (table)
	{
		table->pool = pool;
		table->the_map = map_create(
				hash_basic_integer,
				pool,
				sizeof(object_number_t));
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

return_t obj_add_object(
		object_table_t * const t,
		void * const obj,
		object_number_t * const objno)
{
	bool_t id_ok = false;
	return_t ret = NO_ERROR;

	if (t && obj)
	{
		/* find the next suitable object id to use */
		object_number_t id = t->next_id;
		const uint32_t tc = map_capacity(t->the_map);
		for ( ; id != tc ; id++ )
		{
			if ( !map_contains_key(t->the_map, &id) )
			{
				id_ok = true;
				break;
			}
		}

		/* now create an allocate the object */
		if (id_ok)
		{
			if ( !map_put((t->the_map), &id, obj))
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

return_t obj_remove_object(
		object_table_t * const t,
		const object_number_t objno)
{
	return_t ret = NO_ERROR;

	if (t)
	{
		if (map_contains_key(t->the_map, &objno))
		{
			if(!map_remove(t->the_map, &objno))
			{
				ret = OBJECT_NOT_REMOVED_FROM_REGISTRY;
			}
		}
		else
		{
			ret = INVALID_OBJECT;
		}
	}

	return ret;
}

void * obj_get_object(const object_table_t * const t, object_number_t oid)
{
	object_t * o = NULL;
	if (t)
	{
		object_t * tmp_object = NULL;
		if (map_get(t->the_map, &oid, (void**)&tmp_object))
		{
			o = tmp_object;
		}
	}
	return o;
}

map_it_t * obj_iterator(const object_table_t * t)
{
	return map_it_create(t->the_map);
}
