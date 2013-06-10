/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "object_table.h"

#include "object.h"
#include "obj_thread.h"

#include "../utils/util_malloc.h"
#include "../memory/memory_manager.h"

/** TODO: this takes up too much space mainly because we declare and index list for
 * all the elements (1024), this could be optimised down to only have a table for
 * blocks of say 10 elements.
 * @param object_map_t
 * @param UINT32
 * @param
 * @param __MAX_OBJECT_TABLE_SIZE
 */
HASH_MAP_TYPE_T(object_map_t, uint32_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)
HASH_MAP_SPEC_T(static, object_map_t, uint32_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)
HASH_MAP_BODY_T(static, object_map_t, uint32_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)

error_t __obj_initialse_table(__process_t * proc, __object_table_t * t)
{
	error_t ret = NO_ERROR;
	/* create the hashmap for the object table */
	if ( t )
	{
		t->the_map = __mem_alloc(proc->memory_pool, sizeof(object_map_t));
		t->the_map = object_map_t_create(
				__hash_basic_integer,
				proc->memory_pool);
		if  (t->the_map )
		{
			t->next_id = 1;
		}
		else
		{
			ret = OUT_OF_MEMORY;
		}
	}
	return ret;
}

error_t __obj_allocate_next_free_object(
		__mem_pool_info_t * pool,
		__object_table_t * t,
		__object_t ** o)
{
	bool id_ok = false;
	error_t ret = NO_ERROR;

	if ( t && o)
	{
		object_map_t * map = (object_map_t*)t->the_map;
		/* find the next suitable object id to use */
		uint32_t id = t->next_id;
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
			__object_t * obj = __mem_alloc(pool, sizeof(__object_t));
			if ( object_map_t_put(( (object_map_t*)t->the_map), id, obj) )
			{
				__obj_initialise_object(obj);
				__obj_set_number(obj, id);
				__obj_set_allocated(obj, true);
				*o = obj;
			} else {
				ret = OBJECT_ADD_FAILED;
			}
		} else {
			ret = OBJECT_TABLE_FULL;
		}
	} else {
		ret = PARAMETERS_INVALID;
	}

	return ret;
}

__object_t * __obj_get_thread_object(__object_table_t * t, uint32_t oid)
{
	__object_t * o = NULL;
	if ( t )
	{
		__object_t * tmp_object;
		object_map_t * map = (object_map_t*)t->the_map;

		if ( object_map_t_get(map, oid, &tmp_object) )
		{
			if ( __obj_get_type(tmp_object) == THREAD_OBJ)
			{
				o = tmp_object;
			}
		}
	}
	return o;
}
