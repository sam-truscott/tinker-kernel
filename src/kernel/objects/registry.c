/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "registry.h"

#include "config.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/utils/collections/hashed_map.h"

typedef struct registry_entry
{
	const __process_t * process;
	object_number_t objno;
} registry_entry_t;

typedef char registry_key_t[__MAX_SHARED_OBJECT_NAME_LENGTH];

HASH_MAP_TYPE_T(registry_t)
HASH_MAP_INTERNAL_TYPE_T(registry_t, registry_key_t, registry_entry_t, __MAX_SHARED_OBJECTS)
HASH_MAP_SPEC_T(static, registry_t, registry_key_t, registry_entry_t, __MAX_SHARED_OBJECTS)
HASH_FUNCS_POINTER(registry_t, registry_key_t)
HASH_MAP_BODY_T(static, registry_t, registry_key_t, registry_entry_t, __MAX_SHARED_OBJECTS)

static registry_t * __registry = NULL;

void __registry_initialise(__mem_pool_info_t * const pool)
{
	__registry = registry_t_create(
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
