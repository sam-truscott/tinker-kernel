/*
 *
 * SOS Source Code
 * __________________
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
	const __process_t * process;
	object_number_t objno;
} registry_entry_t;

HASH_MAP_TYPE_T(registry_t)
HASH_MAP_INTERNAL_TYPE_T(registry_t, registry_key_t, registry_entry_t, __MAX_SHARED_OBJECTS, 16)
HASH_MAP_SPEC_T(static, registry_t, registry_key_t, registry_entry_t, __MAX_SHARED_OBJECTS)
HASH_FUNCS_POINTER(registry_t, registry_key_t)
HASH_MAP_BODY_T(static, registry_t, registry_key_t, registry_entry_t, __MAX_SHARED_OBJECTS, 16)

UNBOUNDED_QUEUE_TYPE(waiting_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_CREATE(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_INITIALISE(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_DELETE(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_PUSH(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_POP(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_FRONT(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_CREATE(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_INITIALISE(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_DELETE(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_PUSH(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_POP(static, waiting_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_FRONT(static, waiting_queue_t, __object_thread_t*)


HASH_MAP_TYPE_T(waiting_map_t)
HASH_MAP_INTERNAL_TYPE_T(waiting_map_t, registry_key_t, waiting_queue_t*, __MAX_SHARED_OBJECTS, 1024)
HASH_MAP_SPEC_T(static, waiting_map_t, registry_key_t, waiting_queue_t*, __MAX_SHARED_OBJECTS)
HASH_FUNCS_POINTER(waiting_map_t, registry_key_t)
HASH_MAP_BODY_T(static, waiting_map_t, registry_key_t, waiting_queue_t*, __MAX_SHARED_OBJECTS, 1024)

static registry_t * __registry = NULL;

static waiting_map_t * __registry_waiting_map = NULL;

void __registry_initialise(__mem_pool_info_t * const pool)
{
	__registry = registry_t_create(
			__hash_basic_string,
			__hash_equal_string,
			false,
			pool);

	__registry_waiting_map = waiting_map_t_create(
			__hash_basic_string,
			__hash_equal_string,
			false,
			pool);
}

error_t __regsitery_add(
		const char * const name,
		const __process_t * const process,
		const object_number_t number)
{
	error_t ret = NO_ERROR;
	registry_key_t key;
	memset(key, 0, sizeof(key));
	__util_memcpy(key, name, __util_strlen(name, __MAX_SHARED_OBJECT_NAME_LENGTH));

	if (!registry_t_contains_key(__registry, key))
	{
		registry_entry_t entry = {
				.process = process,
				.objno = number};

		const bool_t added = registry_t_put(__registry, key, entry);
		if (!added)
		{
			ret = OBJECT_NOT_ADDED_TO_REGISTRY;
		}
		else
		{
			if (waiting_map_t_contains_key(__registry_waiting_map, key))
			{
				waiting_queue_t * queue = NULL;
				if (waiting_map_t_get(__registry_waiting_map, key, &queue))
				{
					if (queue)
					{
						__object_thread_t * thread = NULL;
						while (waiting_queue_t_front(queue, &thread))
						{
							if (thread)
							{
								__obj_set_thread_ready(thread);
							}
							waiting_queue_t_pop(queue);
						}
						waiting_map_t_remove(__registry_waiting_map, key);
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

error_t __registry_get(
		const char * const name,
		__process_t ** process,
		object_number_t * objno)
{

	error_t ret = NO_ERROR;
	registry_key_t key;
	memset(key, 0, sizeof(key));
	__util_memcpy(key, name, __util_strlen(name, __MAX_SHARED_OBJECT_NAME_LENGTH));

	if (registry_t_contains_key(__registry, key))
	{
		registry_entry_t entry = {
						.process = NULL,
						.objno = 0};
		const bool_t retrieved = registry_t_get(__registry, key, &entry);
		if (retrieved)
		{
			*process = (__process_t*)entry.process;
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
	return ret;
}

void __registry_wait_for(
		__object_thread_t * const thread,
		const char * const name)
{
	__obj_set_thread_waiting(thread, NULL);

	registry_key_t key;
	memset(key, 0, sizeof(key));
	__util_memcpy(key, name, __util_strlen(name, __MAX_SHARED_OBJECT_NAME_LENGTH));

	waiting_queue_t * queue = NULL;
	if (!waiting_map_t_contains_key(__registry_waiting_map, key))
	{
		queue = waiting_queue_t_create(__registry_waiting_map->pool);
		if (queue)
		{
			waiting_map_t_put(__registry_waiting_map, key, queue);
		}
	}
	else
	{
		waiting_map_t_get(__registry_waiting_map, key, &queue);
	}

	if (queue)
	{
		waiting_queue_t_push(queue, thread);
	}
}

error_t __registry_remove(const char * const name)
{
	error_t ret = NO_ERROR;

	registry_key_t key;
	memset(key, 0, sizeof(key));
	__util_memcpy(key, name, __util_strlen(name, __MAX_SHARED_OBJECT_NAME_LENGTH));

	if (registry_t_contains_key(__registry, key))
	{
		const bool_t removed = registry_t_remove(__registry, key);
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
