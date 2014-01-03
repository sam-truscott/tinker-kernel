/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "config.h"
#include "obj_shared_mem.h"
#include "arch/tgt_types.h"
#include "object_private.h"
#include "registry.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/collections/unbounded_list.h"

UNBOUNDED_LIST_TYPE(shm_client_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_CREATE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_DELETE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_ADD(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_GET(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_REMOVE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_REMOVE_ITEM(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_SPEC_SIZE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_CREATE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_DELETE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_ADD(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_GET(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_REMOVE(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static,shm_client_list_t, __object_shm_t*)
UNBOUNDED_LIST_BODY_SIZE(static,shm_client_list_t, __object_shm_t*)

typedef struct __object_shm_t
{
	__object_internal_t object;
	__mem_pool_info_t * pool;
	__process_t * process;
	uint32_t real_addr;
	uint32_t virt_addr;
	uint32_t size;
	shm_client_list_t * client_list;
	struct __object_shm_t * parent_shm;
	char name[__MAX_SHARED_OBJECT_NAME_LENGTH];
} __object_shm_internal_t;

__object_shm_t * __obj_cast_shm(__object_t * const o)
{
	__object_shm_t * shm = NULL;
	if (o)
	{
		const __object_shm_t * const tmp = (__object_shm_t*)o;
		if (tmp->object.type == SHARED_MEMORY_OBJ)
		{
			shm = (__object_shm_t*)tmp;
		}
	}
	return shm;
}

object_number_t __obj_shm_get_oid
	(const __object_shm_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

error_t __obj_create_shm(
		__process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address)
{
	error_t result = NO_ERROR;
	if (process && objectno && address && size && name)
	{
		__object_shm_t * no = NULL;
		__object_table_t * const table = __process_get_object_table(process);
		__mem_pool_info_t * const pool = __process_get_mem_pool(process);
		no = (__object_shm_t*)__mem_alloc(pool, sizeof(__object_shm_t));
		if (no)
		{
			const void * const memory = __mem_alloc_aligned(pool, size, MMU_PAGE_SIZE);
			if (memory)
			{
				object_number_t objno;
				result = __obj_add_object(table, (__object_t*)no, &objno);
				if (result == NO_ERROR)
				{
					uint32_t virt_addr;
					result =
							__process_allocate_vmem(
									process,
									(uint32_t)memory,
									size,
									MMU_RANDOM_ACCESS_MEMORY,
									MMU_USER_ACCESS,
									MMU_READ_WRITE,
									&virt_addr);
					if (result == NO_ERROR)
					{
						__obj_initialise_object(&no->object, objno, SHARED_MEMORY_OBJ);
						no->size = size;
						no->pool = pool;
						no->process = process;
						no->real_addr = (uint32_t)memory;
						no->virt_addr = virt_addr;
						no->client_list = shm_client_list_t_create(pool);
						no->parent_shm = NULL;
						memset(no->name, 0, __MAX_SHARED_OBJECT_NAME_LENGTH);
						__util_memcpy(no->name, name, __util_strlen(name, __MAX_SHARED_OBJECT_NAME_LENGTH));
						__regsitery_add(name, process, no->object.object_number);
						*objectno = no->object.object_number;
						*address = (void*)virt_addr;
					}
					else
					{
						result = OUT_OF_MEMORY;
					}
				}
			}
			else
			{
				__mem_free(pool, no);
				result = OUT_OF_MEMORY;
			}
		}
		else
		{
			result = OUT_OF_MEMORY;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}

error_t __obj_open_shm(
		__process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address)
{
	error_t result = NO_ERROR;
	if (process && objectno && name && size)
	{
		__object_shm_t * no = NULL;
		__object_table_t * const table = __process_get_object_table(process);
		__mem_pool_info_t * const pool = __process_get_mem_pool(process);
		no = (__object_shm_t*)__mem_alloc(pool, sizeof(__object_shm_t));
		if (no)
		{
			__process_t * other_process = NULL;
			object_number_t other_obj_no;
			if (__registry_get(name, &other_process, &other_obj_no) == NO_ERROR)
			{
				__object_t * const other_obj =
						__obj_get_object(__process_get_object_table(other_process), other_obj_no);
				if (other_obj)
				{
					__object_shm_t * const other_shm_obj = __obj_cast_shm(other_obj);
					if (other_shm_obj)
					{
						if (size == other_shm_obj->size)
						{
							uint32_t virt_addr;
							result = __process_allocate_vmem(
											process,
											other_shm_obj->real_addr,
											other_shm_obj->size,
											MMU_RANDOM_ACCESS_MEMORY,
											MMU_USER_ACCESS,
											MMU_READ_WRITE,
											&virt_addr);
							if (result == NO_ERROR)
							{
								object_number_t objno;
								result = __obj_add_object(table, (__object_t*)no, &objno);
								if (result == NO_ERROR)
								{
									__obj_initialise_object(&no->object, objno, SHARED_MEMORY_OBJ);
									no->size = size;
									no->pool = pool;
									no->process = process;
									no->real_addr = other_shm_obj->real_addr;
									no->virt_addr = virt_addr;
									no->parent_shm = other_shm_obj;
									shm_client_list_t_add(other_shm_obj->client_list, no);
									memset(no->name, 0, sizeof(no->name));
									__util_memcpy(no->name, name, __util_strlen(name, sizeof(name)));
									__regsitery_add(name, process, no->object.object_number);
									*objectno = no->object.object_number;
									*address = (void*)virt_addr;
								}
								else
								{
									result = OUT_OF_MEMORY;
								}
							}
							else
							{
								result = OUT_OF_MEMORY;
							}
						}
						else
						{
							result = SHM_WRONG_SIZE;
						}
					}
					else
					{
						result = WRONG_OBJ_TYPE;
					}
				}
				else
				{
					result = INVALID_OBJECT;
				}
			}
			else
			{
				result = OBJECT_NOT_IN_REGISTRY;
			}
		}
		else
		{
			result = OUT_OF_MEMORY;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}

error_t __obj_delete_shm(
		__object_shm_t * const shm)
{
	error_t result = NO_ERROR;

	if (shm)
	{
		// this is an owner, it may have children that
		// are still attached and we need to close them first
		if (shm->client_list)
		{
			uint32_t size = shm_client_list_t_size(shm->client_list);
			while(size)
			{
				__object_shm_t * attached_shm = NULL;
				if (shm_client_list_t_get(shm->client_list, 0, &attached_shm))
				{
					if (attached_shm)
					{
						__obj_delete_shm(attached_shm);
					}
				}
				shm_client_list_t_remove(shm->client_list, 0);
				size = shm_client_list_t_size(shm->client_list);
			}
			shm_client_list_t_delete(shm->client_list);
			__process_free_vmem(shm->process, shm->virt_addr);
			__mem_free(shm->pool, (void*)shm->real_addr);
			__registry_remove(shm->name);
			__mem_free(shm->pool, shm);
		}
		else
		{
			__process_free_vmem(shm->process, shm->virt_addr);
			if (shm->parent_shm)
			{
				shm_client_list_t_remove_item(shm->parent_shm->client_list, shm);
			}
			__mem_free(shm->pool, shm);
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}
