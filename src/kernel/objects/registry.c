/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "registry.h"

#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/utils/collections/hashed_map.h"
#include "kernel/utils/collections/unbounded_queue.h"

typedef struct registry_entry
{
	const process_t * process;
	object_number_t objno;
} registry_entry_t;

typedef char registry_key_t[MAX_SHARED_OBJECT_NAME_LENGTH];

HASH_MAP_TYPE_T(registry_t)
HASH_MAP_INTERNAL_TYPE_T(registry_t, registry_key_t, registry_entry_t, MAX_SHARED_OBJECTS, 16)
HASH_MAP_SPEC_CREATE(static, registry_t)
HASH_MAP_SPEC_INITALISE(static, registry_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, registry_t, registry_key_t)
HASH_MAP_SPEC_PUT(static, registry_t, registry_key_t, registry_entry_t)
HASH_MAP_SPEC_GET(static, registry_t, registry_key_t, registry_entry_t)
HASH_MAP_SPEC_REMOVE(static, registry_t, registry_key_t, registry_entry_t)
HASH_FUNCS_POINTER(registry_t, registry_key_t)
HASH_MAP_BODY_CREATE(static, registry_t)
HASH_MAP_BODY_INITALISE(static, registry_t, MAX_SHARED_OBJECTS, 16)
HASH_MAP_BODY_CONTAINS_KEY(static, registry_t, registry_key_t, 16)
HASH_MAP_BODY_PUT(static, registry_t, registry_key_t, registry_entry_t, MAX_SHARED_OBJECTS, 16)
HASH_MAP_BODY_GET(static, registry_t, registry_key_t, registry_entry_t, 16)
HASH_MAP_BODY_REMOVE(static, registry_t, registry_key_t, 16)

UNBOUNDED_QUEUE_TYPE(waiting_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_SPEC_CREATE(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_SPEC_INITIALISE(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_SPEC_DELETE(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_SPEC_PUSH(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_SPEC_POP(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_SPEC_FRONT(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_BODY_CREATE(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_BODY_INITIALISE(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_BODY_DELETE(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_BODY_PUSH(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_BODY_POP(static, waiting_queue_t, object_thread_t*)
UNBOUNDED_QUEUE_BODY_FRONT(static, waiting_queue_t, object_thread_t*)


HASH_MAP_TYPE_T(waiting_map_t)
HASH_MAP_INTERNAL_TYPE_T(waiting_map_t, registry_key_t, waiting_queue_t*, MAX_SHARED_OBJECTS, 1024)
HASH_MAP_SPEC_CREATE(static, waiting_map_t)
HASH_MAP_SPEC_INITALISE(static, waiting_map_t)
HASH_MAP_SPEC_CONTAINS_KEY(static, waiting_map_t, registry_key_t)
HASH_MAP_SPEC_PUT(static, waiting_map_t, registry_key_t, waiting_queue_t*)
HASH_MAP_SPEC_GET(static, waiting_map_t, registry_key_t, waiting_queue_t*)
HASH_MAP_SPEC_REMOVE(static, waiting_map_t, registry_key_t, waiting_queue_t*)
HASH_FUNCS_POINTER(waiting_map_t, registry_key_t)
HASH_MAP_BODY_CREATE(static, waiting_map_t)
HASH_MAP_BODY_INITALISE(static, waiting_map_t, MAX_SHARED_OBJECTS, 1024)
HASH_MAP_BODY_CONTAINS_KEY(static, waiting_map_t, registry_key_t, 1024)
HASH_MAP_BODY_PUT(static, waiting_map_t, registry_key_t, waiting_queue_t*, MAX_SHARED_OBJECTS, 1024)
HASH_MAP_BODY_GET(static, waiting_map_t, registry_key_t, waiting_queue_t*, 1024)
HASH_MAP_BODY_REMOVE(static, waiting_map_t, registry_key_t, 1024)

static registry_t * registry = NULL;

static waiting_map_t * registry_waiting_map = NULL;

void registry_initialise(mem_pool_info_t * const pool)
{
#if defined(REGISTRY_DEBUGGING)
	debug_print("Registry: Initialising...\n");
#endif
	registry = registry_t_create(
			hash_basic_string,
			hash_equal_string,
			false,
			pool);

	registry_waiting_map = waiting_map_t_create(
			hash_basic_string,
			hash_equal_string,
			false,
			pool);
}

error_t regsitery_add(
		const char * const name,
		const process_t * const process,
		const object_number_t number)
{
#if defined(REGISTRY_DEBUGGING)
	debug_print("Registry: Adding entry %s with object %d from process %x\n", name, number, process);
#endif
	error_t ret = NO_ERROR;
	registry_key_t key;
	util_memset(key, 0, sizeof(key));
	util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

	if (!registry_t_contains_key(registry, key))
	{
		registry_entry_t entry = {
				.process = process,
				.objno = number};

		const bool_t added = registry_t_put(registry, key, entry);
		if (!added)
		{
			ret = OBJECT_NOT_ADDED_TO_REGISTRY;
		}
		else
		{
			if (waiting_map_t_contains_key(registry_waiting_map, key))
			{
				waiting_queue_t * queue = NULL;
				if (waiting_map_t_get(registry_waiting_map, key, &queue))
				{
					if (queue)
					{
						object_thread_t * thread = NULL;
						while (waiting_queue_t_front(queue, &thread))
						{
							if (thread)
							{
								obj_set_thread_ready(thread);
							}
							waiting_queue_t_pop(queue);
						}
						waiting_map_t_remove(registry_waiting_map, key);
						waiting_queue_t_delete(queue);
					}
				}
			}
		}
	}
	else
	{
		ret = OBJECT_NAME_EXISTS_IN_REGISTRY;
	}
	return ret;
}

error_t registry_get(
		const char * const name,
		process_t ** process,
		object_number_t * objno)
{
#if defined(REGISTRY_DEBUGGING)
	debug_print("Registry: Getting entry %s\n", name);
#endif
	error_t ret = NO_ERROR;
	registry_key_t key;
	util_memset(key, 0, sizeof(key));
	util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

	if (registry_t_contains_key(registry, key))
	{
		registry_entry_t entry = {
						.process = NULL,
						.objno = 0};
		const bool_t retrieved = registry_t_get(registry, key, &entry);
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
#if defined(REGISTRY_DEBUGGING)
	debug_print("Registry: Get entry %s, ret %d, objno %d\n", name, ret, *objno);
#endif
	return ret;
}

void registry_wait_for(
		object_thread_t * const thread,
		const char * const name)
{
#if defined(REGISTRY_DEBUGGING)
	debug_print("Registry: Thread %s is waiting for %s\n", thread_get_name(obj_get_thread(thread)), name);
#endif
	obj_set_thread_waiting(thread, NULL);

	registry_key_t key;
	util_memset(key, 0, sizeof(key));
	util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

	waiting_queue_t * queue = NULL;
	if (!waiting_map_t_contains_key(registry_waiting_map, key))
	{
		queue = waiting_queue_t_create(registry_waiting_map->pool);
		if (queue)
		{
			waiting_map_t_put(registry_waiting_map, key, queue);
		}
	}
	else
	{
		waiting_map_t_get(registry_waiting_map, key, &queue);
	}

	if (queue)
	{
		waiting_queue_t_push(queue, thread);
	}
}

error_t registry_remove(const char * const name)
{
#if defined(REGISTRY_DEBUGGING)
	debug_print("Registry: Removing entry %s\n", name);
#endif
	error_t ret = NO_ERROR;

	registry_key_t key;
	util_memset(key, 0, sizeof(key));
	util_memcpy(key, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));

	if (registry_t_contains_key(registry, key))
	{
		const bool_t removed = registry_t_remove(registry, key);
		if (!removed)
		{
			ret = OBJECT_NOT_REMOVED_FROM_REGISTRY;
		}
	}
	else
	{
		ret = OBJECT_NOT_IN_REGISTRY;
	}
	return ret;
}
