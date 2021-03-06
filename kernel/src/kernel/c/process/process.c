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
#include "console/print_out.h"
#include "utils/util_memset.h"
#include "utils/util_memcpy.h"

static void process_add_mem_sec(
		process_t * const process,
		mem_section_t * const section)
{
	if (process)
	{
		if (is_debug_enabled(PROCESS))
		{
			debug_print(PROCESS, "Process: Adding new section V:%8x:%8x -> R:%8x:%8x (%x) [%s]\n",
					mem_sec_get_virt_addr(section),
					mem_sec_get_virt_addr(section) + mem_sec_get_size(section),
					mem_sec_get_real_addr(section),
					mem_sec_get_real_addr(section) + mem_sec_get_size(section),
					mem_sec_get_size(section),
					mem_sec_get_name(section));
		}
		if (!process->first_section)
		{
			debug_prints(MEMORY, "Process: Section is first section\n");
			process->first_section = section;
		}
		else
		{
			debug_prints(MEMORY, "Process: Inserting at correct place\n");
			mem_section_t * s = process->first_section;
			mem_section_t * p = NULL;
			bool_t assigned = false;
			while (s && !assigned)
			{
				if (mem_sec_get_virt_addr(section) < mem_sec_get_virt_addr(s))
				{
					if (is_debug_enabled(PROCESS))
					{
						debug_print(MEMORY, "Process: Inserting before %x -> %x\n", mem_sec_get_virt_addr(s), mem_sec_get_real_addr(s));
					}
					mem_sec_set_next(section, s);
					if (p != NULL)
					{
						if (is_debug_enabled(PROCESS))
						{
							debug_print(MEMORY, "Process: Inserting after %x -> %x\n", mem_sec_get_virt_addr(p), mem_sec_get_real_addr(p));
						}
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
				if (is_debug_enabled(PROCESS))
				{
					debug_print(MEMORY, "Process: Inserting at end %x -> %x\n", mem_sec_get_virt_addr(section), mem_sec_get_real_addr(section));
				}
				mem_sec_set_next(p, section);
			}
		}
		if (is_debug_enabled(PROCESS_TRACE))
		{
			debug_prints(PROCESS_TRACE, "-------------------------------\n");
			mem_section_t * s = process->first_section;
			while (s)
			{
#if defined (KERNEL_DEBUGGING)
				if (is_debug_enabled(PROCESS))
				{
					const mem_t sz = mem_sec_get_size(s);
					const mem_t virt = mem_sec_get_virt_addr(s);
					const mem_t real = mem_sec_get_real_addr(s);
					debug_print(PROCESS_TRACE, "%x:%x->%x:%x\n", virt, virt + sz, real, real + sz);
				}
#endif
				s = mem_sec_get_next(s);
			}
			debug_prints(PROCESS_TRACE, "-------------------------------\n");
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
						(mem_t)mem_alloc(new_proc->parent_pool, PRIVATE_POOL_SIZE),
						PRIVATE_POOL_SIZE,
						&new_proc->private_pool));
		if (is_debug_enabled(PROCESS))
		{
			debug_print(PROCESS, "Private pool allocated space is %d before page table\n", mem_get_allocd_size(new_proc->private_pool));
		}
		new_proc->page_table = tgt_initialise_page_table(new_proc->private_pool);
		if (is_debug_enabled(PROCESS))
		{
			debug_print(PROCESS, "Private pool allocated space is %d after page table\n", mem_get_allocd_size(new_proc->private_pool));
		}
		kernel_assert("new process's pool wasn't created from private pool", new_proc->page_table != NULL);

		debug_print(PROCESS, "Process: Allocating memory for page table: %s\n", name);
		for (int i = 0 ; i < MAX_THREADS ; i++)
		{
			new_proc->threads[i] = NULL;
		}
		const uint32_t length = util_strlen(name,MAX_PROCESS_IMAGE_LEN);
		util_memcpy(new_proc->image, name, length);
		new_proc->image[length] = '\0';
		new_proc->object_table = obj_table_create(new_proc->private_pool);
		new_proc->initial_thread = NULL;
		new_proc->first_section = NULL;

		mem_t end_of_ram = bsp_get_usable_memory_end();
		// FIXME this doesn't look right - should be doing kernel mapping
		// for the code but into the wrong place
		mem_section_t * const kernel_ram_sec = mem_sec_create(
				new_proc->private_pool,
				4096, 		/* real (exclude first page) */
				0,    		/* virtual ( #### this will hit the apps #### ) */
				end_of_ram, /* size */
				MMU_RANDOM_ACCESS_MEMORY,
				MMU_KERNEL_ACCESS,
				MMU_READ_WRITE,
				"RAM");
		/* kernel access for the process - mapped but not as a section */
		// FIXME: memory leak on kernel_ram_sec
		tgt_map_memory(new_proc, kernel_ram_sec);
		while (ksection)
		{
			mem_t real = mem_sec_get_real_addr(ksection);
			// FIXME doesn't take into account app space so will map
			// other apps
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
								mem_sec_get_access(ksection),
								mem_sec_get_name(ksection)));
			}
			ksection = mem_sec_get_next(ksection);
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
					MMU_READ_WRITE,
					"POOL (PRIVATE)"));

		/* create a section for the heap - allocated from the private pool */
		process_add_mem_sec(
				new_proc,
				mem_sec_create(
					new_proc->private_pool,
					mem_get_start_addr(new_proc->memory_pool),
					VIRTUAL_ADDRESS_SPACE(is_kernel) + PRIVATE_POOL_SIZE,
					mem_get_alloc_size(new_proc->memory_pool),
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_ALL_ACCESS,
					MMU_READ_WRITE,
					"POOL (PUBLIC)"));

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
						(mmu_memory_t)(part->mem_type),
						(mmu_privilege_t)(part->priv),
						(mmu_access_t)(part->access),
						part->name));
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
					MMU_ALL_ACCESS,
                    MMU_READ_ONLY,
					"SYSCALL API"));

		debug_print(PROCESS, "Process: Calling target initialisation for process: %s\n", name);
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

static uint32_t count_threads(process_t * const process, uint32_t * const id)
{
	bool_t found = false;

	uint32_t threads = 0;
	if (process)
	{
		for (int i = 0 ; i < MAX_THREADS ; i++)
		{
			if (process->threads[i] != NULL)
			{
				threads++;
			}
			if (process->threads[i] == NULL && !found && id != NULL)
			{
				*id = i;
				found = true;
			}
		}
	}
	return threads;
}

bool_t process_add_thread(
		process_t * const process,
		thread_t * const thread,
		object_number_t * const objno)
{
	kernel_assert("process is null", process != NULL);
	bool_t ret = false;

	uint32_t thread_id = 0;
	const uint32_t thread_count = count_threads(process, &thread_id);

	if ( thread_count < MAX_THREADS )
	{
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
			process->threads[thread_id] = thread;
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

uint32_t process_get_thread_count(const process_t * const process)
{
	kernel_assert("process is null", process != NULL);
	return count_threads((process_t *)process, NULL);
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
	uint32_t tid = thread_get_tid(thread);
	thread_t * const t = process->threads[tid];
	if (t)
	{
		process->threads[tid] = NULL;
	}
}

void process_exit(process_t * const process)
{
	kernel_assert("process_exit - check process is not null", process != NULL);
	// thread list - process should only die when this is empty
	for (uint32_t i = 0 ; i < MAX_THREADS ; i++)
	{
		process->threads[i] = NULL;
	}
	// object table
	map_it_t * const it = obj_iterator(process->object_table);
	if (it)
	{
		object_t * object = NULL;
		bool_t objects_exist = map_it_get(it, (void**)&object);
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
			objects_exist = map_it_next(it, (void**)&object);
		}
		map_it_reset(it);
		kernel_assert("object table isn't empty", map_it_get(it, (void**)&object) == false);
		map_it_delete(it);
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

mem_t process_virt_to_real(
		const process_t * const process,
		const mem_t virt)
{
	mem_t real = virt;
	if (process)
	{
		bool_t found = false;
		mem_section_t * current = process->first_section;
		debug_print(PROCESS, "Process: First section at %x\n", current);
		while (current && !found)
		{
			const mem_t vstart = mem_sec_get_virt_addr(current);
			const mem_t vsize = mem_sec_get_size(current);
			const mem_t vend = vstart + vsize;
			debug_print(PROCESS, "Process: Checking for %x between %x and %x\n", virt, vstart, vend);
			if (virt >= vstart && virt <= vend)
			{
				const mem_t offset = virt - vstart;
				const mem_t vreal = mem_sec_get_real_addr(current);
				real = vreal + offset;
				debug_print(PROCESS, "Process: Found %x in v=%x -> r=%x with offset %x (%x)\n", virt, vstart, vreal, offset, real);
				found = true;
			}
			current = mem_sec_get_next(current);
		}
		if (!found)
		{
			const mem_t pool_start = mem_get_start_addr(process->memory_pool);
			const mem_t base = VIRTUAL_ADDRESS_SPACE(process->kernel_process);
			debug_print(PROCESS, "Process: Unable to find a mem section, falling back: %x. Pool start %x, Base: %x\n", virt, pool_start, base);
			if (real >= base)
			{
				real -= base;
			}
			real += pool_start;
		}
	}
	return real;
}

return_t process_allocate_vmem(
		process_t * const process,
		const mem_t real_address,
		const mem_t size,
		const mmu_memory_t type,
		const mmu_privilege_t priv,
		const mmu_access_t access,
		mem_t * const virt_address,
		const char * const name)
{
	return_t result = NO_ERROR;
	// find a hole in the memory sections that's large enough
	// and then allocate a memory section there
	// note: this is just a virtual allocation to avoid
	// collisions in the memory address space of this process
	*virt_address = 0;
	if (process)
	{
		mem_t vmem_start = VIRTUAL_ADDRESS_SPACE(process->kernel_process);
		mem_section_t * current = process->first_section;
		mem_section_t * prev = NULL;
		while (current)
		{
			const mem_t svt = mem_sec_get_virt_addr(current);
			if (svt > VIRTUAL_ADDRESS_SPACE(process->kernel_process) && svt > (vmem_start + size))
			{
				// there's room
				mem_section_t * const new_section =
						mem_sec_create(process->private_pool, real_address, vmem_start, size, type, priv, access, name);
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
					mem_sec_create(process->private_pool, real_address, vmem_start, size, type, priv, access, name);
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
		const mem_t virt_address)
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

thread_t * thread_by_index(const process_t * const process, uint32_t index)
{
	thread_t * it = NULL;
	if (process && index < MAX_THREADS)
	{
		it = process->threads[index];
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
