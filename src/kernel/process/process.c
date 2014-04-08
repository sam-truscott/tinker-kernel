/*
 *
 * SOS Source Code
 * __________________
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

HASH_MAP_TYPE_T(__thread_map_t)
HASH_MAP_INTERNAL_TYPE_T(__thread_map_t, uint32_t, __thread_t*, __MAX_THREADS, 16)
HASH_MAP_SPEC_CREATE(static, __thread_map_t)
HASH_MAP_SPEC_INITALISE(static, __thread_map_t)
HASH_MAP_SPEC_SIZE(static, __thread_map_t, uint32_t, __thread_t*, __MAX_THREADS)
HASH_MAP_SPEC_CONTAINS_KEY(static, __thread_map_t, uint32_t)
HASH_MAP_SPEC_PUT(static, __thread_map_t, uint32_t, __thread_t*)
HASH_MAP_SPEC_REMOVE(static, __thread_map_t, uint32_t, __thread_t*)
HASH_MAP_SPEC_DELETE(static, __thread_map_t)
HASH_FUNCS_VALUE(__thread_map_t, uint32_t)
HASH_MAP_BODY_CREATE(static, __thread_map_t)
HASH_MAP_BODY_INITALISE(static, __thread_map_t, __MAX_THREADS, 16)
HASH_MAP_BODY_SIZE(static, __thread_map_t)
HASH_MAP_BODY_CONTAINS_KEY(static, __thread_map_t, uint32_t, 16)
HASH_MAP_BODY_PUT(static, __thread_map_t, uint32_t, __thread_t*, __MAX_THREADS, 16)
HASH_MAP_BODY_REMOVE(static, __thread_map_t, uint32_t, 16)
HASH_MAP_BODY_DELETE(static, __thread_map_t, __MAX_THREADS, 16)

HASH_MAP_TYPE_ITERATOR_INTERNAL_TYPE(__thread_it_t, __thread_map_t)
HASH_MAP_TYPE_ITERATOR_BODY(extern, __thread_it_t, __thread_map_t, uint32_t, __thread_t*, __MAX_THREADS, 16)

typedef struct __process_t
{
	uint32_t				process_id;
	__thread_map_t	*		threads;
	__mem_pool_info_t * 	memory_pool;
	__object_table_t *		object_table;
	object_number_t			object_number;
	bool_t					kernel_process;
	tgt_mem_t				segment_info;
	__mem_section_t *			first_section;
	char					image[__MAX_PROCESS_IMAGE_LEN + 1];
	uint32_t 				next_thread_id;
	__thread_t * 			initial_thread;
	__mem_pool_info_t * 	parent;
	tgt_pg_tbl_t *			page_table;
} __process_internal_t;

static void __process_add_mem_sec(
		__process_t * const process,
		__mem_section_t * const section)
{
	if (process)
	{
		if (!process->first_section)
		{
			process->first_section = section;
		}
		else
		{
			__mem_section_t * s = process->first_section;
			__mem_section_t * p = NULL;
			bool_t assigned = false;
			while (s)
			{
				if (__mem_sec_get_virt_addr(section) < __mem_sec_get_virt_addr(s))
				{
					__mem_sec_set_next(section, s);
					if (p != NULL)
					{
						__mem_sec_set_next(p, section);
					}
					if (s == process->first_section)
					{
						process->first_section = section;
					}
					__mem_sec_set_next(section,s);
					assigned = true;
					break;
				}
				p = s;
				s = __mem_sec_get_next(s);
			}
			if (!assigned)
			{
				__mem_sec_set_next(p, section);
			}
		}
	}
}

error_t __process_create(
		__mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool_t is_kernel,
		const sos_meminfo_t * const meminfo,
		const __mem_pool_info_t * pool,
		__process_t ** process)
{
	__process_t * p = (__process_t*)__mem_alloc(mempool, sizeof(__process_t));
	error_t ret = NO_ERROR;
	if (p)
	{
		p->threads = __thread_map_t_create(
				__hash_basic_integer,
				__hash_equal_integer,
				true,
				mempool);
		p->process_id = pid;
		p->kernel_process = is_kernel;
		p->parent = mempool;
		p->memory_pool = (__mem_pool_info_t *)pool;
		p->page_table = (tgt_pg_tbl_t*)__mem_alloc_aligned(p->memory_pool, PAGE_TABLE_SIZE, PAGE_TABLE_ALIGNMENT);
		if (p->page_table)
		{
		    memset(p->page_table, 0, PAGE_TABLE_SIZE);
		}
		const uint32_t length = __util_strlen(name,__MAX_PROCESS_IMAGE_LEN);
		__util_memcpy(p->image, name, length);
		p->image[length] = '\0';
		p->object_table = __obj_table_create(p->memory_pool);
		p->next_thread_id = 0;
		p->initial_thread = NULL;

		__process_add_mem_sec(
				p,
				__mem_sec_create(
					p->memory_pool,
					__mem_get_start_addr(p->memory_pool),
					VIRTUAL_ADDRESS_SPACE,
					__mem_get_alloc_size(p->memory_pool),
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_USER_ACCESS,
					MMU_READ_WRITE));

		__process_add_mem_sec(
				p,
				__mem_sec_create(
					p->memory_pool,
				    meminfo->text_start,
					meminfo->text_start,
					meminfo->text_size,
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_USER_ACCESS,
					MMU_READ_ONLY));

		__process_add_mem_sec(
                p,
                __mem_sec_create(
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

        __process_add_mem_sec(
                p,
                __mem_sec_create(
                    p->memory_pool,
                    (const uint32_t)api,
                    (const uint32_t)api,
                    (const uint32_t)(eapi - api),
                    MMU_RANDOM_ACCESS_MEMORY,
                    MMU_USER_ACCESS,
                    MMU_READ_ONLY));

		ret = __tgt_initialise_process(p);
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

uint32_t __process_get_pid(const __process_t * const process)
{
	return process->process_id;
}

const char * __process_get_image(const __process_t * const process)
{
	return process->image;
}

object_number_t __process_get_oid(const __process_t * const process)
{
	return process->object_number;
}

void __process_set_oid(
		__process_t * const process,
		const object_number_t oid)
{
	process->object_number = oid;
}

bool_t __process_is_kernel(const __process_t * const process)
{
	return process->kernel_process;
}

__mem_pool_info_t * __process_get_mem_pool(const __process_t * const process)
{
	return process->memory_pool;
}

__object_table_t * __process_get_object_table(const __process_t * const process)
{
	return process->object_table;
}

const tgt_mem_t * __process_get_segment_info(const __process_t * const process)
{
	return &process->segment_info;
}

void __process_set_segment_info(
		__process_t * const process,
		const tgt_mem_t * const seg)
{
	__util_memcpy(
			&process->segment_info,
			seg,
			sizeof(tgt_mem_t));
}

bool_t __process_add_thread(
		__process_t * const process,
		__thread_t * const thread,
		object_number_t * const objno)
{
	bool_t ret = false;
	const uint32_t thread_count = __thread_map_t_size(process->threads);
	if ( thread_count < __MAX_THREADS )
	{
		uint32_t thread_id = (process->next_thread_id % __MAX_THREADS);
		for ( uint32_t i = thread_id ; i < __MAX_THREADS ; i++ )
		{
			if ( !__thread_map_t_contains_key(process->threads, thread_id) )
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
		__thread_set_tid(thread, thread_id);

		// create an object in the table
		ret = __obj_create_thread(
				process->memory_pool,
				process->object_table,
				thread_id,
				thread,
				 objno);

		if (ret)
		{
			__thread_set_oid(thread, *objno);
			if(!__thread_map_t_put(process->threads, thread_id, thread))
			{
				ret = OUT_OF_MEMORY;
			}
		}
	}
	return ret;
}

__thread_t * __process_get_main_thread(const __process_t * process)
{
	return process->initial_thread;
}

uint32_t __process_get_thread_count(const __process_t * process)
{
	return __thread_map_t_size(process->threads);
}

const __mem_section_t * __process_get_first_section(const __process_t * const process)
{
	return process->first_section;
}

void __process_thread_exit(__process_t * const process, __thread_t * const thread)
{
	__kernel_assert("__process_thread_exit - check process is not null", process != NULL);
	__kernel_assert("__process_thread_exit - check thread is not null", thread != NULL);
	__thread_map_t_remove(process->threads, __thread_get_tid(thread));
}

void __process_exit(__process_t * const process)
{
	__kernel_assert("process_exit - check process is not null", process != NULL);
	// thread list
	__thread_map_t_delete(process->threads);
	// object table
	__object_table_it_t * const it = __obj_iterator(process->object_table);
	if (it)
	{
		__object_t * object = NULL;
		bool_t objects_exist = __object_table_it_t_get(it, &object);
		while (objects_exist)
		{
			// delete it
			__object_thread_t * const thread = __obj_cast_thread(object);
			if (thread)
			{
				if (!(__thread_get_flags(__obj_get_thread(thread)) & THREAD_FLAG_TIMER))
				{
					const object_number_t oid = __obj_thread_get_oid(thread);
					__obj_exit_thread(thread);
					__obj_remove_object(process->object_table, oid);
				}
				else
				{
					__kernel_assert("hi", false);
				}
			}
			else
			{
				__object_sema_t * const sema = __obj_cast_semaphore(object);
				if (sema)
				{
					const object_number_t oid = __obj_semaphore_get_oid(sema);
					__object_delete_semaphore(sema);
					__obj_remove_object(
							process->object_table,
							oid);
				}
				else
				{
					__object_process_t * const process_obj = __obj_cast_process(object);
					if (process_obj)
					{
						const object_number_t oid = __obj_process_get_oid(process_obj);
						__obj_delete_process(process_obj);
						__obj_remove_object(
								process->object_table,
								oid);
					}
					else
					{
						__object_pipe_t * const pipe_obj = __obj_cast_pipe(object);
						if (pipe_obj)
						{
							const object_number_t oid = __obj_pipe_get_oid(pipe_obj);
							__obj_delete_pipe(pipe_obj);
							__obj_remove_object(
									process->object_table,
									oid);
						}
						else
						{
							__object_shm_t * const shm_obj = __obj_cast_shm(object);
							if (shm_obj)
							{
								const object_number_t oid = __obj_shm_get_oid(shm_obj);
								__obj_delete_shm(shm_obj);
								__obj_remove_object(
										process->object_table,
										oid);
							}
							else
							{
								__object_timer_t * const timer_obj = __obj_cast_timer(object);
								if (timer_obj)
								{
									const object_number_t oid = __obj_timer_get_oid(timer_obj);
									__obj_delete_timer(timer_obj);
									__obj_remove_object(
											process->object_table,
											oid);
								}
							}
						}
					}
				}
			}
			objects_exist = __object_table_it_t_next(it, &object);
		}
		__object_table_it_t_reset(it);
		__kernel_assert("object table isn't empty", __object_table_it_t_get(it, &object) == false);
		__object_table_it_t_delete(it);
	}
	__obj_table_delete(process->object_table);

	// memory sections
	const __mem_section_t * section = process->first_section;
	while (section)
	{
		__tgt_unmap_memory(process, section);
		__mem_sec_delete(section);
		section = __mem_sec_get_next(section);
	}

	__mem_free(process->parent, process->memory_pool);
	__mem_free(process->parent, process);
}

uint32_t __process_virt_to_real(
		const __process_t * const process,
		const uint32_t virt)
{
	uint32_t real = virt;
	if (process)
	{
		/* FIXME TODO should scan the sections rather than assume */
		const uint32_t pool_start = __mem_get_start_addr(process->memory_pool);
		real -= VIRTUAL_ADDRESS_SPACE;
		real += pool_start;
	}
	return real;
}

error_t __process_allocate_vmem(
		__process_t * const process,
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
		__mem_section_t * current = process->first_section;
		__mem_section_t * prev = NULL;
		while (current)
		{
			const uint32_t svt = __mem_sec_get_virt_addr(current);
			if (svt > VIRTUAL_ADDRESS_SPACE && svt > (vmem_start + size))
			{
				// there's room
				__mem_section_t * const new_section =
						__mem_sec_create(process->memory_pool, real_address, vmem_start, size, type, priv, access);
				if (new_section)
				{
					// insert into list
					if (prev)
					{
						__mem_sec_set_next(prev, new_section);
					}
					if (!prev)
					{
						process->first_section = new_section;
					}
					__mem_sec_set_next(new_section, current);
					result = __tgt_map_memory(process, new_section);
					*virt_address = vmem_start;
				}
				break;
			}
			else
			{
				vmem_start = __mem_sec_get_virt_addr(current) + __mem_sec_get_size(current);

				// pad each region out by mmu page size
				vmem_start += MMU_PAGE_SIZE;
			}
			prev = current;
			current = __mem_sec_get_next(current);
		}
		if (!current && prev && !(*virt_address))
		{
			__mem_section_t * const new_section =
					__mem_sec_create(process->memory_pool, real_address, vmem_start, size, type, priv, access);
			__mem_sec_set_next(prev, new_section);
			*virt_address = vmem_start;
			result = __tgt_map_memory(process, new_section);
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}

void __process_free_vmem(
		const __process_t * const process,
		const uint32_t virt_address)
{
	if (process)
	{
		__mem_section_t * current = process->first_section;
		__mem_section_t * prev = NULL;
		while (current)
		{
			if (__mem_sec_get_virt_addr(current) == virt_address)
			{
				// insert into list
				if (prev)
				{
					__mem_sec_set_next(prev, __mem_sec_get_next(current));
				}
				__tgt_unmap_memory(process, current);
				__mem_sec_delete(current);
				break;
			}
			prev = current;
			current = __mem_sec_get_next(current);
		}
	}
}

__thread_it_t * __process_iterator(const __process_t * const process)
{
	return __thread_it_t_create(process->threads);
}

const tgt_pg_tbl_t * __process_get_page_table(const __process_t * const process)
{
	return process->page_table;
}
