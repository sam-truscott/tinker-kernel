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
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/collections/hashed_map.h"

HASH_MAP_TYPE_T(thread_map_t)
HASH_MAP_INTERNAL_TYPE_T(thread_map_t, uint32_t, __thread_t*, __MAX_THREADS, 16)
HASH_MAP_SPEC_T(static, thread_map_t, uint32_t, __thread_t*, __MAX_THREADS)
HASH_FUNCS_VALUE(thread_map_t, uint32_t)
HASH_MAP_BODY_T(static, thread_map_t, uint32_t,__thread_t*, __MAX_THREADS, 16)

typedef struct __process_t
{
	uint32_t				process_id;
	thread_map_t	*		threads;
	__mem_pool_info_t * 	memory_pool;
	__object_table_t *		object_table;
	object_number_t			object_number;
	bool_t					kernel_process;
	tgt_mem_t				segment_info;
	const mem_section_t *	first_section;
	char					image[__MAX_PROCESS_IMAGE_LEN + 1];
	uint32_t 				next_thread_id;
	__thread_t * 			initial_thread;
	__mem_pool_info_t * 	parent;
} __process_internal_t;

error_t __process_create(
		__mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool_t is_kernel,
		const __mem_pool_info_t * pool,
		__process_t ** process)
{
	__process_t * p = (__process_t*)__mem_alloc(mempool, sizeof(__process_t));
	error_t ret = NO_ERROR;
	if (p)
	{
		p->threads = thread_map_t_create(
				__hash_basic_integer,
				__hash_equal_integer,
				true,
				mempool);
		p->process_id = pid;
		p->kernel_process = is_kernel;
		p->parent = mempool;
		p->memory_pool = (__mem_pool_info_t *)pool;
		const uint32_t length = __util_strlen(name,__MAX_PROCESS_IMAGE_LEN);
		__util_memcpy(p->image, name, length);
		p->image[length] = '\0';
		p->object_table = __obj_table_create(p->memory_pool);
		p->next_thread_id = 0;
		p->initial_thread = NULL;

		p->first_section = __mem_sec_create(
				p->memory_pool,
				__mem_get_start_addr(p->memory_pool),
				VIRTUAL_ADDRESS_SPACE,
				__mem_get_alloc_size(p->memory_pool),
				mmu_random_access_memory,
				mmu_user_access,
				mmu_read_write);

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
	const uint32_t thread_count = thread_map_t_size(process->threads);
	if ( thread_count < __MAX_THREADS )
	{
		uint32_t thread_id = (process->next_thread_id % __MAX_THREADS);
		for ( uint32_t i = thread_id ; i < __MAX_THREADS ; i++ )
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
		__thread_set_tid(thread, thread_id);

		// create an object in the table
		ret = __obj_create_thread(
				process->memory_pool,
				process->object_table,
				process->process_id,
				thread_id,
				thread,
				process->object_number,
				 objno);

		__thread_set_oid(thread, *objno);

		thread_map_t_put(process->threads, thread_id, thread);

		ret = true;
	}
	return ret;
}

__thread_t * __process_get_main_thread(const __process_t * process)
{
	return process->initial_thread;
}

uint32_t __process_get_thread_count(const __process_t * process)
{
	return thread_map_t_size(process->threads);
}

__thread_t * __process_get_thread(const __process_t * process, const uint32_t tid)
{
	__thread_t * t = NULL;
	thread_map_t_get(process->threads, tid, &t);
	return t;
}

const mem_section_t * __process_get_first_section(const __process_t * const process)
{
	return process->first_section;
}

void __process_thread_exit(__process_t * const process, __thread_t * const thread)
{
	__kernel_assert("__process_thread_exit - check process is not null", process != NULL);
	__kernel_assert("__process_thread_exit - check thread is not null", thread != NULL);
	thread_map_t_remove(process->threads, __thread_get_tid(thread));
}

void __process_exit(__process_t * const process)
{
	__kernel_assert("process_exit - check process is not null", process != NULL);
	// thread list
	thread_map_t_delete(process->threads);
	// object table
	object_table_it_t * const it = __obj_iterator(process->object_table);
	if (it)
	{
		__object_t * object = NULL;
		bool_t objects_exist = object_table_it_t_get(it, &object);
		while (objects_exist)
		{
			// delete it
			__object_thread_t * const thread = __obj_cast_thread(object);
			if (thread)
			{
				const object_number_t oid = __obj_thread_get_oid(thread);
				__obj_exit_thread(thread);
				__obj_remove_object(process->object_table, oid);
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
					}
				}
				/* TODO: Support the other object types */
			}
			object_table_it_t_reset(it);
			objects_exist = object_table_it_t_get(it, &object);
		}
		object_table_it_t_delete(it);
	}
	__obj_table_delete(process->object_table);

	// memory sections
	__tgt_destroy_process(process);
	const mem_section_t * section = process->first_section;
	while (section)
	{
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
