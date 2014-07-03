/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "process.h"

#include "config.h"
#include "arch/tgt.h"
#include "kernel/kernel_assert.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/objects/obj_process.h"
#include "kernel/objects/obj_pipe.h"
#include "kernel/objects/obj_shared_mem.h"
#include "kernel/objects/obj_timer.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/collections/hashed_map.h"

HASH_MAP_TYPE_T(thread_map_t)
HASH_MAP_INTERNAL_TYPE_T(thread_map_t, uint32_t, thread_t*, MAX_THREADS, 16)
HASH_MAP_SPEC_CREATE(static, thread_map_t)
HASH_MAP_SPEC_INITALISE(static, thread_map_t)
HASH_MAP_SPEC_SIZE(static, thread_map_t, uint32_t, thread_t*, MAX_THREADS)
HASH_MAP_SPEC_CONTAINS_KEY(static, thread_map_t, uint32_t)
HASH_MAP_SPEC_PUT(static, thread_map_t, uint32_t, thread_t*)
HASH_MAP_SPEC_REMOVE(static, thread_map_t, uint32_t, thread_t*)
HASH_MAP_SPEC_DELETE(static, thread_map_t)
HASH_FUNCS_VALUE(thread_map_t, uint32_t)
HASH_MAP_BODY_CREATE(static, thread_map_t)
HASH_MAP_BODY_INITALISE(static, thread_map_t, MAX_THREADS, 16)
HASH_MAP_BODY_SIZE(static, thread_map_t)
HASH_MAP_BODY_CONTAINS_KEY(static, thread_map_t, uint32_t, 16)
HASH_MAP_BODY_PUT(static, thread_map_t, uint32_t, thread_t*, MAX_THREADS, 16)
HASH_MAP_BODY_REMOVE(static, thread_map_t, uint32_t, 16)
HASH_MAP_BODY_DELETE(static, thread_map_t, MAX_THREADS, 16)

HASH_MAP_TYPE_ITERATOR_INTERNAL_TYPE(thread_it_t, thread_map_t)
HASH_MAP_TYPE_ITERATOR_BODY(extern, thread_it_t, thread_map_t, uint32_t, thread_t*, MAX_THREADS, 16)

typedef struct process_t
{
	uint32_t				process_id;
	thread_map_t	*		threads;
	mem_pool_info_t * 	memory_pool;
	object_table_t *		object_table;
	object_number_t			object_number;
	bool_t					kernel_process;
	tgt_mem_t				mem_info;
	mem_section_t *		first_section;
	char					image[MAX_PROCESS_IMAGE_LEN + 1];
	uint32_t 				next_thread_id;
	thread_t * 			initial_thread;
	mem_pool_info_t * 	parent;
	tgt_pg_tbl_t *			page_table;
} process_internal_t;

static void process_add_mem_sec(
		process_t * const process,
		mem_section_t * const section)
{
	if (process)
	{
		if (!process->first_section)
		{
			process->first_section = section;
		}
		else
		{
			mem_section_t * s = process->first_section;
			mem_section_t * p = NULL;
			bool_t assigned = false;
			while (s)
			{
				if (mem_sec_get_virt_addr(section) < mem_sec_get_virt_addr(s))
				{
					mem_sec_set_next(section, s);
					if (p != NULL)
					{
						mem_sec_set_next(p, section);
					}
					if (s == process->first_section)
					{
						process->first_section = section;
					}
					mem_sec_set_next(section,s);
					assigned = true;
					break;
				}
				p = s;
				s = mem_sec_get_next(s);
			}
			if (!assigned)
			{
				mem_sec_set_next(p, section);
			}
		}
	}
}

error_t process_create(
		mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool_t is_kernel,
		const tinker_meminfo_t * const meminfo,
		const mem_pool_info_t * pool,
		process_t ** process)
{
	process_t * p = (process_t*)mem_alloc(mempool, sizeof(process_t));
	error_t ret = NO_ERROR;
	if (p)
	{
		p->process_id = pid;
		p->kernel_process = is_kernel;
		p->parent = mempool;
		p->memory_pool = (mem_pool_info_t *)pool;
		p->page_table = (tgt_pg_tbl_t*)mem_alloc_aligned(p->memory_pool, PAGE_TABLE_SIZE, PAGE_TABLE_ALIGNMENT);
		if (p->page_table)
		{
		    util_memset(p->page_table, 0, PAGE_TABLE_SIZE);
		}
		p->threads = thread_map_t_create(
		        hash_basic_integer,
		        hash_equal_integer,
		        true,
		        p->memory_pool);
		const uint32_t length = util_strlen(name,MAX_PROCESS_IMAGE_LEN);
		util_memcpy(p->image, name, length);
		p->image[length] = '\0';
		p->object_table = obj_table_create(p->memory_pool);
		p->next_thread_id = 0;
		p->initial_thread = NULL;

		process_add_mem_sec(
				p,
				mem_sec_create(
					p->memory_pool,
					mem_get_start_addr(p->memory_pool),
					VIRTUAL_ADDRESS_SPACE,
					mem_get_alloc_size(p->memory_pool),
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_USER_ACCESS,
					MMU_READ_WRITE));

		process_add_mem_sec(
				p,
				mem_sec_create(
					p->memory_pool,
				    meminfo->text_start,
					meminfo->text_start,
					meminfo->text_size,
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_USER_ACCESS,
					MMU_READ_ONLY));

		process_add_mem_sec(
                p,
                mem_sec_create(
                    p->memory_pool,
                    meminfo->data_start,
                    meminfo->data_start,
                    meminfo->data_size,
                    MMU_RANDOM_ACCESS_MEMORY,
                    MMU_USER_ACCESS,
                    MMU_READ_WRITE));

        extern char * __api;
        extern char * __api_end;
        char * api = (char*)&__api;
        char * eapi = (char*)&__api_end;

        process_add_mem_sec(
                p,
                mem_sec_create(
                    p->memory_pool,
                    (const uint32_t)api,
                    (const uint32_t)api,
                    (const uint32_t)(eapi - api),
                    MMU_RANDOM_ACCESS_MEMORY,
                    MMU_USER_ACCESS,
                    MMU_READ_ONLY));

		ret = tgt_initialise_process(p);
		if (ret == NO_ERROR && process)
		{
			*process = p;
		}
	}
	else
	{
		ret = OUT_OF_MEMORY;
	}
	return ret;
}

uint32_t process_get_pid(const process_t * const process)
{
	return process->process_id;
}

const char * process_get_image(const process_t * const process)
{
	return process->image;
}

object_number_t process_get_oid(const process_t * const process)
{
	return process->object_number;
}

void process_set_oid(
		process_t * const process,
		const object_number_t oid)
{
	process->object_number = oid;
}

bool_t process_is_kernel(const process_t * const process)
{
	return process->kernel_process;
}

mem_pool_info_t * process_get_mem_pool(const process_t * const process)
{
	return process->memory_pool;
}

object_table_t * process_get_object_table(const process_t * const process)
{
	return process->object_table;
}

const tgt_mem_t * process_get_mem_info(const process_t * const process)
{
	return &process->mem_info;
}

void process_set_mem_info(
		process_t * const process,
		const tgt_mem_t * const seg)
{
	util_memcpy(
			&process->mem_info,
			seg,
			sizeof(tgt_mem_t));
}

bool_t process_add_thread(
		process_t * const process,
		thread_t * const thread,
		object_number_t * const objno)
{
	bool_t ret = false;
	const uint32_t thread_count = thread_map_t_size(process->threads);
	if ( thread_count < MAX_THREADS )
	{
		uint32_t thread_id = (process->next_thread_id % MAX_THREADS);
		for ( uint32_t i = thread_id ; i < MAX_THREADS ; i++ )
		{
			if ( !thread_map_t_contains_key(process->threads, thread_id) )
			{
				thread_id = i;
				break;
			}
		}
		process->next_thread_id++;

		if (process->initial_thread == NULL)
		{
			process->initial_thread = thread;
		}

		// update the thread's id
		thread_set_tid(thread, thread_id);

		// create an object in the table
		ret = obj_create_thread(
				process->memory_pool,
				process->object_table,
				thread_id,
				thread,
				 objno);

		if (ret)
		{
			thread_set_oid(thread, *objno);
			if(!thread_map_t_put(process->threads, thread_id, thread))
			{
				ret = OUT_OF_MEMORY;
			}
		}
	}
	return ret;
}

thread_t * process_get_main_thread(const process_t * process)
{
	return process->initial_thread;
}

uint32_t process_get_thread_count(const process_t * process)
{
	return thread_map_t_size(process->threads);
}

const mem_section_t * process_get_first_section(const process_t * const process)
{
	return process->first_section;
}

void process_thread_exit(process_t * const process, thread_t * const thread)
{
	kernel_assert("process_thread_exit - check process is not null", process != NULL);
	kernel_assert("process_thread_exit - check thread is not null", thread != NULL);
	thread_map_t_remove(process->threads, thread_get_tid(thread));
}

void process_exit(process_t * const process)
{
	kernel_assert("process_exit - check process is not null", process != NULL);
	// thread list
	thread_map_t_delete(process->threads);
	// object table
	object_table_it_t * const it = obj_iterator(process->object_table);
	if (it)
	{
		object_t * object = NULL;
		bool_t objects_exist = object_table_it_t_get(it, &object);
		while (objects_exist)
		{
			// delete it
			object_thread_t * const thread = obj_cast_thread(object);
			if (thread)
			{
				if (!(thread_get_flags(obj_get_thread(thread)) & THREAD_FLAG_TIMER))
				{
					const object_number_t oid = obj_thread_get_oid(thread);
					obj_exit_thread(thread);
					obj_remove_object(process->object_table, oid);
				}
			}
			else
			{
				object_sema_t * const sema = obj_cast_semaphore(object);
				if (sema)
				{
					const object_number_t oid = obj_semaphore_get_oid(sema);
					object_delete_semaphore(sema);
					obj_remove_object(
							process->object_table,
							oid);
				}
				else
				{
					object_process_t * const process_obj = obj_cast_process(object);
					if (process_obj)
					{
						const object_number_t oid = obj_process_get_oid(process_obj);
						obj_delete_process(process_obj);
						obj_remove_object(
								process->object_table,
								oid);
					}
					else
					{
						object_pipe_t * const pipe_obj = obj_cast_pipe(object);
						if (pipe_obj)
						{
							const object_number_t oid = obj_pipe_get_oid(pipe_obj);
							obj_delete_pipe(pipe_obj);
							obj_remove_object(
									process->object_table,
									oid);
						}
						else
						{
							object_shm_t * const shm_obj = obj_cast_shm(object);
							if (shm_obj)
							{
								const object_number_t oid = obj_shm_get_oid(shm_obj);
								obj_delete_shm(shm_obj);
								obj_remove_object(
										process->object_table,
										oid);
							}
							else
							{
								object_timer_t * const timer_obj = obj_cast_timer(object);
								if (timer_obj)
								{
									const object_number_t oid = obj_timer_get_oid(timer_obj);
									obj_delete_timer(timer_obj);
									obj_remove_object(
											process->object_table,
											oid);
								}
							}
						}
					}
				}
			}
			objects_exist = object_table_it_t_next(it, &object);
		}
		object_table_it_t_reset(it);
		kernel_assert("object table isn't empty", object_table_it_t_get(it, &object) == false);
		object_table_it_t_delete(it);
	}
	obj_table_delete(process->object_table);

	// memory sections
	const mem_section_t * section = process->first_section;
	while (section)
	{
	    const mem_section_t * const tmp = section;
		tgt_unmap_memory(process, section);
		section = mem_sec_get_next(section);
		mem_sec_delete(tmp);
	}

	mem_free(process->memory_pool, process->page_table);
	mem_free(process->parent, process->memory_pool);
	mem_free(process->parent, process);
}

uint32_t process_virt_to_real(
		const process_t * const process,
		const uint32_t virt)
{
	uint32_t real = virt;
	if (process)
	{
		/* FIXME TODO should scan the sections rather than assume */
		const uint32_t pool_start = mem_get_start_addr(process->memory_pool);
		real -= VIRTUAL_ADDRESS_SPACE;
		real += pool_start;
	}
	return real;
}

error_t process_allocate_vmem(
		process_t * const process,
		const uint32_t real_address,
		const uint32_t size,
		const mmu_memory_t type,
		const mmu_privilege_t priv,
		const mmu_access_t access,
		uint32_t * const virt_address)
{
	error_t result = NO_ERROR;
	// find a hole in the memory sections that's large enough
	// and then allocate a memory section there
	// note: this is just a virtual allocation to avoid
	// collisions in the memory address space of this process
	*virt_address = 0;
	if (process)
	{
		uint32_t vmem_start = VIRTUAL_ADDRESS_SPACE;
		mem_section_t * current = process->first_section;
		mem_section_t * prev = NULL;
		while (current)
		{
			const uint32_t svt = mem_sec_get_virt_addr(current);
			if (svt > VIRTUAL_ADDRESS_SPACE && svt > (vmem_start + size))
			{
				// there's room
				mem_section_t * const new_section =
						mem_sec_create(process->memory_pool, real_address, vmem_start, size, type, priv, access);
				if (new_section)
				{
					// insert into list
					if (prev)
					{
						mem_sec_set_next(prev, new_section);
					}
					if (!prev)
					{
						process->first_section = new_section;
					}
					mem_sec_set_next(new_section, current);
					result = tgt_map_memory(process, new_section);
					*virt_address = vmem_start;
				}
				break;
			}
			else
			{
				vmem_start = mem_sec_get_virt_addr(current) + mem_sec_get_size(current);

				// pad each region out by mmu page size
				vmem_start += MMU_PAGE_SIZE;
			}
			prev = current;
			current = mem_sec_get_next(current);
		}
		if (!current && prev && !(*virt_address))
		{
			mem_section_t * const new_section =
					mem_sec_create(process->memory_pool, real_address, vmem_start, size, type, priv, access);
			mem_sec_set_next(prev, new_section);
			*virt_address = vmem_start;
			result = tgt_map_memory(process, new_section);
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}

void process_free_vmem(
		const process_t * const process,
		const uint32_t virt_address)
{
	if (process)
	{
		mem_section_t * current = process->first_section;
		mem_section_t * prev = NULL;
		while (current)
		{
			if (mem_sec_get_virt_addr(current) == virt_address)
			{
				// insert into list
				if (prev)
				{
					mem_sec_set_next(prev, mem_sec_get_next(current));
				}
				tgt_unmap_memory(process, current);
				mem_sec_delete(current);
				break;
			}
			prev = current;
			current = mem_sec_get_next(current);
		}
	}
}

thread_it_t * process_iterator(const process_t * const process)
{
	return thread_it_t_create(process->threads);
}

const tgt_pg_tbl_t * process_get_page_table(const process_t * const process)
{
	return process->page_table;
}
