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
#include "utils/util_memset.h"
#include "utils/collections/hashed_map.h"
#include "utils/collections/unbounded_queue.h"
#include "utils/hash/basic_hashes.h"
#include "console/print_out.h"

typedef struct registry_entry
{
	const process_t * process;
	object_number_t objno;
} registry_entry_t;

typedef char registry_key_t[MAX_SHARED_OBJECT_NAME_LENGTH];

typedef struct registry_t
{
	mem_pool_info_t * pool;
	map_t * registry;
	map_t * registry_waiting_map;
} registry_t;

registry_t * registry_create(mem_pool_info_t * const pool)
{
	registry_t * reg = mem_alloc(pool, sizeof(registry_t));
	if (reg)
	{
		debug_prints(REGISTRY, "Registry: Initialising...\n");
		reg->registry = map_create(
				hash_basic_string,
				pool,
				sizeof(registry_key_t));
		reg->registry_waiting_map = map_create(
				hash_basic_string,
				pool,
				sizeof(registry_key_t));
		reg->pool = pool;
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

		if (!map_contains_key(reg->registry, key))
		{
			registry_entry_t * entry = mem_alloc(reg->pool, sizeof(registry_entry_t));
			util_memset(entry, 0, sizeof(registry_entry_t));
			entry->process = process;
			entry->objno = number;

			const bool_t added = map_put(reg->registry, key, entry);
			if (!added)
			{
				ret = OBJECT_NOT_ADDED_TO_REGISTRY;
			}
			else
			{
				if (map_contains_key(reg->registry_waiting_map, key))
				{
					queue_t * queue = NULL;
					if (map_get(reg->registry_waiting_map, key, (void*)(&queue)))
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
							map_remove(reg->registry_waiting_map, key);
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

		if (map_contains_key(reg->registry, key))
		{
			registry_entry_t  * entry;
			const bool_t retrieved = map_get(reg->registry, key, (void*)&entry);
			if (retrieved)
			{
				*process = (process_t*)entry->process;
				*objno = entry->objno;
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
		if (!map_contains_key(reg->registry_waiting_map, key))
		{
			queue = queue_create(reg->pool);
			if (queue)
			{
				map_put(reg->registry_waiting_map, key, queue);
			}
		}
		else
		{
			map_get(reg->registry_waiting_map, key, (void*)(&queue));
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

		if (map_contains_key(reg->registry, key))
		{
			registry_entry_t  * entry;
			const bool_t retrieved = map_get(reg->registry, key, (void*)&entry);
			if (retrieved)
			{
				const bool_t removed = map_remove(reg->registry, key);
				if (!removed)
				{
					ret = OBJECT_NOT_REMOVED_FROM_REGISTRY;
				}
				mem_free(reg->pool, entry);
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
