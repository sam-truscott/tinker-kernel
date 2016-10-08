/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "process/process.h"
#include "process/process_private.h"

#include "config.h"
#include "tgt.h"
#include "kernel_assert.h"
#include "board_support.h"
#include "objects/object_table.h"
#include "objects/obj_thread.h"
#include "objects/obj_semaphore.h"
#include "objects/obj_process.h"
#include "objects/obj_pipe.h"
#include "objects/obj_shared_mem.h"
#include "objects/obj_timer.h"
#include "utils/util_strlen.h"

HASH_MAP_TYPE_ITERATOR_INTERNAL_TYPE(thread_it_t, thread_map_t)
HASH_MAP_TYPE_ITERATOR_BODY(extern, thread_it_t, thread_map_t, uint32_t, thread_t*, MAX_THREADS, 16)

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

return_t process_create(
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager,
		mem_pool_info_t * const mempool,
		const uint32_t pid,
		const char * const name,
		const bool_t is_kernel,
		tinker_meminfo_t * const meminfo,
		mem_pool_info_t * pool,
		const mem_section_t * ksection,
		process_t ** process)
{
	process_t * new_proc = (process_t*)mem_alloc(mempool, sizeof(process_t));
	return_t ret = NO_ERROR;
	if (new_proc)
	{
		util_memset(new_proc, 0, sizeof(process_t));
		new_proc->process_id = pid;
		new_proc->kernel_process = is_kernel;
		new_proc->parent_pool = mempool;
		new_proc->scheduler = scheduler;
		new_proc->alarm_manager = alarm_manager;
		new_proc->memory_pool = pool;

		/* allocate a private (kernel) pool for page tables etc */
		kernel_assert(
				"private process pool created ok",
				mem_init_memory_pool(
						(uint32_t)mem_alloc(new_proc->parent_pool, PRIVATE_POOL_SIZE),
						PRIVATE_POOL_SIZE,
						&new_proc->private_pool));
		debug_print(PROCESS, "Private pool allocated space is %d before page table\n", mem_get_allocd_size(new_proc->private_pool));
		new_proc->page_table = tgt_initialise_page_table(new_proc->private_pool);
		debug_print(PROCESS, "Private pool allocated space is %d after page table\n", mem_get_allocd_size(new_proc->private_pool));
		kernel_assert("new process's pool wasn't created from private pool", new_proc->page_table != NULL);

		debug_print(PROCESS, "Process: Allocating memory for page table: %s\n", name);
		new_proc->threads = thread_map_t_create(
		        hash_basic_integer,
		        hash_equal_integer,
		        true,
		        new_proc->private_pool);
		const uint32_t length = util_strlen(name,MAX_PROCESS_IMAGE_LEN);
		util_memcpy(new_proc->image, name, length);
		new_proc->image[length] = '\0';
		new_proc->object_table = obj_table_create(new_proc->private_pool);
		new_proc->next_thread_id = 0;
		new_proc->initial_thread = NULL;
		new_proc->first_section = NULL;

		if (!is_kernel)
		{
			mem_t end_of_ram = bsp_get_usable_memory_end();
			mem_section_t * const kernel_ram_sec = mem_sec_create(
					new_proc->private_pool,
					4096,
					0,
					end_of_ram,
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_KERNEL_ACCESS,
					MMU_READ_WRITE);
			tgt_map_memory(new_proc, kernel_ram_sec);
			while (ksection)
			{
				uint32_t real = mem_sec_get_real_addr(ksection);
				if (real > end_of_ram)
				{
					process_add_mem_sec(
							new_proc,
							mem_sec_create(
									new_proc->private_pool,
									real,
									mem_sec_get_virt_addr(ksection),
									mem_sec_get_size(ksection),
									mem_sec_get_mem_type(ksection),
									mem_sec_get_priv(ksection),
									mem_sec_get_access(ksection)));
				}
				ksection = mem_sec_get_next(ksection);
			}
		}

		/* create a section for the private memory pool */
		process_add_mem_sec(
						new_proc,
						mem_sec_create(
							new_proc->private_pool,
							mem_get_start_addr(new_proc->private_pool),
							VIRTUAL_ADDRESS_SPACE(is_kernel),
							mem_get_alloc_size(new_proc->private_pool),
							MMU_RANDOM_ACCESS_MEMORY,
							MMU_KERNEL_ACCESS,
							MMU_READ_WRITE));

		/* create a section for the heap - allocated from the private pool */
		process_add_mem_sec(
				new_proc,
				mem_sec_create(
					new_proc->private_pool,
					mem_get_start_addr(new_proc->memory_pool),
					VIRTUAL_ADDRESS_SPACE(is_kernel) + PRIVATE_POOL_SIZE,
					mem_get_alloc_size(new_proc->memory_pool),
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_KERNEL_ACCESS, // FIXME why did this work?
					MMU_READ_WRITE));

		tinker_mempart_t * part = meminfo->first_part;
		while (part)
		{
			process_add_mem_sec(
					new_proc,
					mem_sec_create(
						new_proc->private_pool,
						part->real,
						part->virt,
						part->size,
						part->mem_type,
						part->priv,
						part->access));
			part = part->next;
		}

        extern char * __api;
        extern char * __api_end;
        char * api = (char*)&__api;
        char * eapi = (char*)&__api_end;

        /* create a memory for the SYSCALL code - allocated from the private pool */
        process_add_mem_sec(
                new_proc,
                mem_sec_create(
                    new_proc->private_pool,
                    (const uint32_t)api,
                    (const uint32_t)api,
                    (const uint32_t)(eapi - api),
                    MMU_RANDOM_ACCESS_MEMORY,
                    MMU_USER_ACCESS,
                    MMU_READ_ONLY));

		debug_print(PROCESS, "Process: Calling target initialisation for processs: %s\n", name);
		ret = tgt_initialise_process(new_proc);
		if (ret == NO_ERROR && process)
		{
			*process = new_proc;
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
	uint32_t pid = 0;
	if (process)
	{
		pid = process->process_id;
	}
	return pid;
}

const char * process_get_image(const process_t * const process)
{
	const char * image = NULL;
	if (process)
	{
		image = process->image;
	}
	return image;
}

object_number_t process_get_oid(const process_t * const process)
{
	return process->object_number;
}

void process_set_oid(
		process_t * const process,
		const object_number_t oid)
{
	kernel_assert("process is null", process != NULL);
	process->object_number = oid;
}

bool_t process_is_kernel(const process_t * const process)
{
	bool_t is_kernel = false;
	if (process)
	{
		is_kernel = process->kernel_process;
	}
	return is_kernel;
}

mem_pool_info_t * process_get_mem_pool(const process_t * const process)
{
	mem_pool_info_t * pool = NULL;
	if (process)
	{
		pool = process->private_pool;
	}
	return pool;
}

mem_pool_info_t * process_get_user_mem_pool(const process_t * const process)
{
	return process == NULL ? NULL : process->memory_pool;
}

object_table_t * process_get_object_table(const process_t * const process)
{
	object_table_t * table = NULL;
	if (process)
	{
		table = process->object_table;
	}
	return table;
}

const tgt_mem_t * process_get_mem_info(const process_t * const process)
{
	const tgt_mem_t * mem_info = NULL;
	if (process)
	{
		mem_info = &process->mem_info;
	}
	return mem_info;
}

void process_set_mem_info(
		process_t * const process,
		const tgt_mem_t * const seg)
{
	kernel_assert("process is null", process != NULL);
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
	kernel_assert("process is null", process != NULL);
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
				process->private_pool,
				process->scheduler,
				process->alarm_manager,
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
	thread_t * initial = NULL;
	if (process)
	{
		initial = process->initial_thread;
	}
	return initial;
}

uint32_t process_get_thread_count(const process_t * process)
{
	kernel_assert("process is null", process != NULL);
	return thread_map_t_size(process->threads);
}

const mem_section_t * process_get_first_section(const process_t * const process)
{
	mem_section_t * mem_section = NULL;
	if (process)
	{
		mem_section = process->first_section;
	}
	return mem_section;
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
	// thread list - process should only die when this is empty
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

	// memory sections - contained iin the private_pool
	const mem_section_t * section = process->first_section;
	while (section)
	{
	    const mem_section_t * const tmp = section;
		tgt_unmap_memory(process, section);
		section = mem_sec_get_next(section);
		mem_sec_delete(tmp);
	}

	mem_free(process->private_pool, process->page_table);
	mem_free(process->parent_pool, process->private_pool);
	mem_free(process->parent_pool, process->memory_pool);
	mem_free(process->parent_pool, process);
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
		real -= VIRTUAL_ADDRESS_SPACE(process->kernel_process);
		real += pool_start;
	}
	return real;
}

return_t process_allocate_vmem(
		process_t * const process,
		const uint32_t real_address,
		const uint32_t size,
		const mmu_memory_t type,
		const mmu_privilege_t priv,
		const mmu_access_t access,
		uint32_t * const virt_address)
{
	return_t result = NO_ERROR;
	// find a hole in the memory sections that's large enough
	// and then allocate a memory section there
	// note: this is just a virtual allocation to avoid
	// collisions in the memory address space of this process
	*virt_address = 0;
	if (process)
	{
		uint32_t vmem_start = VIRTUAL_ADDRESS_SPACE(process->kernel_process);
		mem_section_t * current = process->first_section;
		mem_section_t * prev = NULL;
		while (current)
		{
			const uint32_t svt = mem_sec_get_virt_addr(current);
			if (svt > VIRTUAL_ADDRESS_SPACE(process->kernel_process) && svt > (vmem_start + size))
			{
				// there's room
				mem_section_t * const new_section =
						mem_sec_create(process->private_pool, real_address, vmem_start, size, type, priv, access);
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
				// FIXME test for 4k alignment
				vmem_start += (vmem_start % MMU_PAGE_SIZE);
			}
			prev = current;
			current = mem_sec_get_next(current);
		}
		if (!current && prev && !(*virt_address))
		{
			vmem_start += (vmem_start % MMU_PAGE_SIZE);
			mem_section_t * const new_section =
					mem_sec_create(process->private_pool, real_address, vmem_start, size, type, priv, access);
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
	thread_it_t * it = NULL;
	if (process)
	{
		it = thread_it_t_create(process->threads);
	}
	return it;
}

tgt_pg_tbl_t * process_get_page_table(const process_t * const process)
{
	tgt_pg_tbl_t * table = NULL;
	if (process)
	{
		table = process->page_table;
	}
	return table;
}
