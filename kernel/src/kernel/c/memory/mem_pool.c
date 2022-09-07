/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "memory/mem_pool.h"

#include "tinker_api_types.h"
#include "tgt_mem.h"
#include "kernel_assert.h"
#include "console/print_out.h"
#include "memory/dlmalloc/malloc.h"

/*
 * this structure is used to manage the blocks
 * stored on the heap
 */
typedef struct mem_pool_info_t
{
	/* dlmalloc memory space */
	mspace space;
	/* total heap in bytes */
	mem_t pool_alloc_size;
} mem_pool_info_internal_t;

bool_t 	mem_init_memory_pool(
		const mem_t base_addr,
		const mem_t pool_size,
		mem_pool_info_t ** const pool)
{
	mem_pool_info_t * pool_info = NULL;

	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: creating new pool at 0x%x for %d bytes\n", base_addr, pool_size);
	}

	/*
	 * put the first info block after the heap at the beginning
	 * of the next page. this gives access to the full heap
	 */
	pool_info = (mem_pool_info_t*)(base_addr);
	pool_info->pool_alloc_size = pool_size;
	if (is_debug_enabled(MEMORY))
	{
		debug_prints(MEMORY, "mem: creating mspace\n");
	}
	pool_info->space = create_mspace_with_base(pool_info + sizeof(mem_pool_info_t), pool_size - sizeof(mem_pool_info_t), 0);

	if (is_debug_enabled(MEMORY))
	{
		debug_prints(MEMORY, "mem: mspace created\n");
	}
	if (pool)
	{
		*pool = pool_info;
	}

	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: new pool is at %x\n", pool_info);
	}

	return true;
}

bool_t	mem_init_process_memory(
		mem_pool_info_t * const pool,
		mem_pool_info_t ** const proc_memory_block,
		const uint32_t size)
{
	bool_t ret = false;

	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: initalising process pool %x size %x\n", pool, size);
	}

	/* allocate that from RAM */
	const mem_t proc_memory_pool = (mem_t)mem_alloc_aligned(
			pool,
			size + MMU_PAGE_SIZE,
			MMU_PAGE_SIZE);

	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: initalised process pool %x size %x result %x\n", pool, size, proc_memory_pool);
	}

	if (proc_memory_pool != 0)
	{
		/* create a table for the given process */
		ret = mem_init_memory_pool(
				proc_memory_pool,
				size - MMU_PAGE_SIZE,
				proc_memory_block);
	}
	else
	{
		ret = false;
	}

	return ret;
}

void *	mem_alloc(
		mem_pool_info_t * const pool,
		const mem_t size)
{
	return mem_alloc_aligned(pool, size, 0);
}

void *  mem_realloc(
		mem_pool_info_t * const pool,
		void * mem,
		const mem_t size)
{
	kernel_assert("mem: attempt to allocate to a null pool\n", pool != NULL);
	void * const new_base = mspace_realloc(pool->space, mem, size);
	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: base at 0x%x, size 0x%x, now 0x%x\n", mem, size, new_base);
	}
	return new_base;
}

void* mem_alloc_aligned(
		mem_pool_info_t * const pool,
		const mem_t size,
		const mem_t alignment)
{
	void* new_base = NULL;

	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: alloc %d bytes (align 0x%x) in pool 0x%x (t=%X/u=%X/f=%X)\n", size, alignment, pool,
				mspace_mallinfo(pool->space).usmblks,
				mspace_mallinfo(pool->space).uordblks,
				mspace_mallinfo(pool->space).fordblks);
	}

	kernel_assert("mem: attempt to allocate to a null pool\n", pool != NULL);
	if (alignment == 0)
	{
		new_base = mspace_malloc(pool->space, size);
	}
	else
	{
		new_base = mspace_memalign(pool->space, alignment, size);
	}

	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: base at 0x%x, size 0x%x\n", new_base, size);
	}

	return new_base;
}

void mem_free(
		mem_pool_info_t * const pool,
		const void * const base)
{
	if (is_debug_enabled(MEMORY))
	{
		debug_print(MEMORY, "mem: free pool 0x%x, base 0x%x\n", pool, base);
	}
	mspace_free(pool->space, (void*)base);
}

mem_t mem_get_start_addr(const mem_pool_info_t * const pool)
{
	return (mem_t)(pool);
}

mem_t mem_get_alloc_size(const mem_pool_info_t * const pool)
{
	mem_t size;
	if (pool)
	{
		size = pool->pool_alloc_size;
	}
	else
	{
		size = 0;
	}
	return size;
}


#pragma GCC diagnostic ignored "-Waggregate-return"

mem_t mem_get_allocd_size(const mem_pool_info_t * const pool)
{
	return mspace_mallinfo(pool->space).uordblks;
}
