/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/obj_shared_mem.h"
#include "config.h"
#include "tgt_types.h"
#include "objects/object_private.h"
#include "utils/util_strlen.h"
#include "utils/collections/unbounded_list.h"

UNBOUNDED_LIST_TYPE(shm_client_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_CREATE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_DELETE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_ADD(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_GET(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_REMOVE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_REMOVE_ITEM(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_SPEC_SIZE(static,shm_client_list_t)
UNBOUNDED_LIST_BODY_CREATE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_DELETE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_ADD(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_GET(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_REMOVE(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static,shm_client_list_t, object_shm_t*)
UNBOUNDED_LIST_BODY_SIZE(static,shm_client_list_t)

typedef struct object_shm_t
{
	object_internal_t object;
	mem_pool_info_t * pool;
	registry_t * reg;
	process_t * process;
	uint32_t real_addr;
	uint32_t virt_addr;
	uint32_t size;
	shm_client_list_t * client_list;
	struct object_shm_t * parent_shm;
	char name[MAX_SHARED_OBJECT_NAME_LENGTH];
} object_shm_internal_t;

object_shm_t * obj_cast_shm(object_t * const o)
{
	object_shm_t * shm = NULL;
	if (o)
	{
		const object_shm_t * const tmp = (object_shm_t*)o;
		if (tmp->object.type == SHARED_MEMORY_OBJ)
		{
			shm = (object_shm_t*)tmp;
		}
	}
	return shm;
}

object_number_t obj_shm_get_oid
	(const object_shm_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

error_t obj_create_shm(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address)
{
	error_t result = NO_ERROR;
	if (process && objectno && address && size && name)
	{
		object_shm_t * no = NULL;
		object_table_t * const table = process_get_object_table(process);
		mem_pool_info_t * const pool = process_get_mem_pool(process);
		no = (object_shm_t*)mem_alloc(pool, sizeof(object_shm_t));
		if (no)
		{
			util_memset(no, 0, sizeof(object_shm_t));
			const void * const memory = mem_alloc_aligned(process_get_user_mem_pool(process), size, MMU_PAGE_SIZE);
			if (memory)
			{
				util_memset((void*)memory, 0, size);
				object_number_t objno;
				result = obj_add_object(table, (object_t*)no, &objno);
				if (result == NO_ERROR)
				{
					uint32_t virt_addr;
					result =
							process_allocate_vmem(
									process,
									(uint32_t)memory,
									size,
									MMU_RANDOM_ACCESS_MEMORY,
									MMU_USER_ACCESS,
									MMU_READ_WRITE,
									&virt_addr);
					if (result == NO_ERROR)
					{
						obj_initialise_object(&no->object, objno, SHARED_MEMORY_OBJ);
						no->size = size;
						no->pool = pool;
						no->reg = reg;
						no->process = process;
						no->real_addr = (uint32_t)memory;
						no->virt_addr = virt_addr;
						no->client_list = shm_client_list_t_create(pool);
						no->parent_shm = NULL;
						util_memset(no->name, 0, MAX_SHARED_OBJECT_NAME_LENGTH);
						util_memcpy(no->name, name, util_strlen(name, MAX_SHARED_OBJECT_NAME_LENGTH));
						regsitery_add(reg, name, process, no->object.object_number);
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
				mem_free(pool, no);
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

error_t obj_open_shm(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address)
{
	error_t result = NO_ERROR;
	if (process && objectno && name && size)
	{
		object_shm_t * no = NULL;
		object_table_t * const table = process_get_object_table(process);
		mem_pool_info_t * const pool = process_get_mem_pool(process);
		no = (object_shm_t*)mem_alloc(pool, sizeof(object_shm_t));
		if (no)
		{
			util_memset(no, 0, sizeof(object_shm_t));
			process_t * other_process = NULL;
			object_number_t other_obj_no;
			if (registry_get(reg, name, &other_process, &other_obj_no) == NO_ERROR)
			{
				object_t * const other_obj =
						obj_get_object(process_get_object_table(other_process), other_obj_no);
				if (other_obj)
				{
					object_shm_t * const other_shm_obj = obj_cast_shm(other_obj);
					if (other_shm_obj)
					{
						if (size == other_shm_obj->size)
						{
							uint32_t virt_addr;
							result = process_allocate_vmem(
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
								result = obj_add_object(table, (object_t*)no, &objno);
								if (result == NO_ERROR)
								{
									obj_initialise_object(&no->object, objno, SHARED_MEMORY_OBJ);
									no->size = size;
									no->pool = pool;
									no->reg = reg;
									no->process = process;
									no->real_addr = other_shm_obj->real_addr;
									no->virt_addr = virt_addr;
									no->parent_shm = other_shm_obj;
									no->client_list = NULL;
									shm_client_list_t_add(other_shm_obj->client_list, no);
									util_memset(no->name, 0, sizeof(no->name));
									util_memcpy(no->name, name, util_strlen(name, sizeof(name)));
									// FIXME why are we adding it again?
									regsitery_add(reg, name, process, no->object.object_number);
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

error_t obj_delete_shm(
		object_shm_t * const shm)
{
	error_t result = NO_ERROR;

	if (shm)
	{
		// this is an owner, it may have children that
		// are still attached and we need to close them first
#if defined(SHM_DEBUGGING)
		debug_print("Deleting SHM\n");
#endif
		if (shm->client_list)
		{
			uint32_t size = shm_client_list_t_size(shm->client_list);
#if defined(SHM_DEBUGGING)
			debug_print("SHM has %d clients\n", size);
#endif
			while(size)
			{
				object_shm_t * attached_shm = NULL;
				if (shm_client_list_t_get(shm->client_list, 0, &attached_shm))
				{
					if (attached_shm && attached_shm != shm)
					{
						obj_delete_shm(attached_shm);
					}
				}
				shm_client_list_t_remove(shm->client_list, 0);
				size = shm_client_list_t_size(shm->client_list);
#if defined(SHM_DEBUGGING)
				debug_print("Detached client, %d clients remain\n", size);
#endif
			}
			shm_client_list_t_delete(shm->client_list);
			process_free_vmem(shm->process, shm->virt_addr);
			mem_free(shm->pool, (void*)shm->real_addr);
#if defined(SHM_DEBUGGING)
			debug_print("Removing SHM from registry\n");
#endif
			registry_remove(shm->reg, shm->name);
			mem_free(shm->pool, shm);
		}
		else
		{
			process_free_vmem(shm->process, shm->virt_addr);
			if (shm->parent_shm)
			{
				shm_client_list_t_remove_item(shm->parent_shm->client_list, shm);
			}
			mem_free(shm->pool, shm);
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}
#if defined(SHM_DEBUGGING)
	debug_print("Done\n");
#endif
	return result;
}
