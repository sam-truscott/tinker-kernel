/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/registry.h"

#include "utils/util_strlen.h"
#include "utils/util_memcpy.h"
#include "utils/collections/hashed_map.h"
#include "utils/collections/unbounded_queue.h"

typedef struct registry_entry
{
	const process_t * process;
	object_number_t objno;
} registry_entry_t;

typedef char registry_key_t[MAX_SHARED_OBJECT_NAME_LENGTH];

HASH_MAP_TYPE_T(registry_map_t)
HASH_MAP_INTERNAL_TYPE_T(registry_map_t, registry_key_t, registry_entry_t, MAX_SHARED_OBJECTS, 16)
HASH_MAP_SPEC_CREATE(static, registry_map_t)
HASH_MAP_SPEC_INITALISE(static, registry_map_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, registry_map_t, registry_key_t)
HASH_MAP_SPEC_PUT(static, registry_map_t, registry_key_t, registry_entry_t)
HASH_MAP_SPEC_GET(static, registry_map_t, registry_key_t, registry_entry_t)
HASH_MAP_SPEC_REMOVE(static, registry_map_t, registry_key_t, registry_entry_t)
HASH_FUNCS_POINTER(registry_map_t, registry_key_t)
HASH_MAP_BODY_CREATE(static, registry_map_t)
HASH_MAP_BODY_INITALISE(static, registry_map_t, MAX_SHARED_OBJECTS, 16)
HASH_MAP_BODY_CONTAINS_KEY(static, registry_map_t, registry_key_t, 16)
HASH_MAP_BODY_PUT(static, registry_map_t, registry_key_t, registry_entry_t, MAX_SHARED_OBJECTS, 16)
HASH_MAP_BODY_GET(static, registry_map_t, registry_key_t, registry_entry_t, 16)
HASH_MAP_BODY_REMOVE(static, registry_map_t, registry_key_t, 16)

HASH_MAP_TYPE_T(waiting_map_t)
HASH_MAP_INTERNAL_TYPE_T(waiting_map_t, registry_key_t, queue_t*, MAX_SHARED_OBJECTS, 1024)
HASH_MAP_SPEC_CREATE(static, waiting_map_t)
HASH_MAP_SPEC_INITALISE(static, waiting_map_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, waiting_map_t, registry_key_t)
HASH_MAP_SPEC_PUT(static, waiting_map_t, registry_key_t, queue_t*)
HASH_MAP_SPEC_GET(static, waiting_map_t, registry_key_t, queue_t*)
HASH_MAP_SPEC_REMOVE(static, waiting_map_t, registry_key_t, queue_t*)
HASH_FUNCS_POINTER(waiting_map_t, registry_key_t)
HASH_MAP_BODY_CREATE(static, waiting_map_t)
HASH_MAP_BODY_INITALISE(static, waiting_map_t, MAX_SHARED_OBJECTS, 1024)
HASH_MAP_BODY_CONTAINS_KEY(static, waiting_map_t, registry_key_t, 1024)
HASH_MAP_BODY_PUT(static, waiting_map_t, registry_key_t, queue_t*, MAX_SHARED_OBJECTS, 1024)
HASH_MAP_BODY_GET(static, waiting_map_t, registry_key_t, queue_t*, 1024)
HASH_MAP_BODY_REMOVE(static, waiting_map_t, registry_key_t, 1024)

typedef struct registry_t
{
	registry_map_t * registry;
	waiting_map_t * registry_waiting_map;
} registry_t;

registry_t * registry_create(mem_pool_info_t * const pool)
{
	registry_t * reg = mem_alloc(pool, sizeof(registry_t));
	if (reg)
	{
		debug_prints(REGISTRY, "Registry: Initialising...\n");
		reg->registry = registry_map_t_create(
				hash_basic_string,
				hash_equal_string,
				false,
				pool);

		reg->registry_waiting_map = waiting_map_t_create(
				hash_basic_string,
				hash_equal_string,
				false,
				pool);
	}
	return reg;
}

return_t regsitery_add(
		registry_t * const reg,
		const char * const name,
		const process_t * const process,
		const object_number_t number)
{
	return_t ret = NO_ERROR;
	debug_print(REGISTRY, "Registry: Adding entry %s with object %d from process %x\n", name, number, process);
	if (reg)
	{
		registry_key_t key;
		util_memset(key, 0, sizeof(key));
		util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

		if (!registry_map_t_contains_key(reg->registry, key))
		{
			registry_entry_t entry = {
					.process = process,
					.objno = number};

			const bool_t added = registry_map_t_put(reg->registry, key, entry);
			if (!added)
			{
				ret = OBJECT_NOT_ADDED_TO_REGISTRY;
			}
			else
			{
				if (waiting_map_t_contains_key(reg->registry_waiting_map, key))
				{
					queue_t * queue = NULL;
					if (waiting_map_t_get(reg->registry_waiting_map, key, &queue))
					{
						if (queue)
						{
							object_thread_t * thread = NULL;
							while (queue_front(queue, &thread))
							{
								if (thread)
								{
									obj_set_thread_ready(thread);
								}
								queue_pop(queue);
							}
							waiting_map_t_remove(reg->registry_waiting_map, key);
							queue_delete(queue);
						}
					}
				}
			}
		}
		else
		{
			ret = OBJECT_NAME_EXISTS_IN_REGISTRY;
		}
	}
	else
	{
		ret = PARAMETERS_NULL;
	}
	return ret;
}

return_t registry_get(
		registry_t * const reg,
		const char * const name,
		process_t ** process,
		object_number_t * objno)
{
	debug_print(REGISTRY, "Registry: Getting entry %s\n", name);
	return_t ret = NO_ERROR;
	if (reg)
	{
		registry_key_t key;
		util_memset(key, 0, sizeof(key));
		util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

		if (registry_map_t_contains_key(reg->registry, key))
		{
			registry_entry_t entry = {
							.process = NULL,
							.objno = 0};
			const bool_t retrieved = registry_map_t_get(reg->registry, key, &entry);
			if (retrieved)
			{
				*process = (process_t*)entry.process;
				*objno = entry.objno;
			}
			else
			{
				ret = OBJECT_NOT_RETRIEVED_FROM_REGISTRY;
			}
		}
		else
		{
			ret = OBJECT_NOT_IN_REGISTRY;
		}
		debug_print(REGISTRY, "Registry: Get entry %s, ret %d, objno %d\n", name, ret, *objno);
	}
	else
	{
		ret = PARAMETERS_NULL;
	}
	return ret;
}

void registry_wait_for(
		registry_t * const reg,
		object_thread_t * const thread,
		const char * const name)
{
	debug_print(REGISTRY, "Registry: Thread %s is waiting for %s\n", thread_get_name(obj_get_thread(thread)), name);
	obj_set_thread_waiting(thread, NULL);

	if (reg)
	{
		registry_key_t key;
		util_memset(key, 0, sizeof(key));
		util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

		queue_t * queue = NULL;
		if (!waiting_map_t_contains_key(reg->registry_waiting_map, key))
		{
			queue = queue_create(reg->registry_waiting_map->pool);
			if (queue)
			{
				waiting_map_t_put(reg->registry_waiting_map, key, queue);
			}
		}
		else
		{
			waiting_map_t_get(reg->registry_waiting_map, key, &queue);
		}

		if (queue)
		{
			queue_push(queue, thread);
		}
	}
}

return_t registry_remove(
		registry_t * const reg,
		const char * const name)
{
	debug_print(REGISTRY, "Registry: Removing entry %s\n", name);
	return_t ret = NO_ERROR;

	if (reg)
	{
		registry_key_t key;
		util_memset(key, 0, sizeof(key));
		util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

		if (registry_map_t_contains_key(reg->registry, key))
		{
			const bool_t removed = registry_map_t_remove(reg->registry, key);
			if (!removed)
			{
				ret = OBJECT_NOT_REMOVED_FROM_REGISTRY;
			}
		}
		else
		{
			ret = OBJECT_NOT_IN_REGISTRY;
		}
	}
	else
	{
		ret = PARAMETERS_NULL;
	}
	return ret;
}
