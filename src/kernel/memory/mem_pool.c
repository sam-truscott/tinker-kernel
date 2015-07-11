/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "mem_pool.h"

#include "tinker_api_types.h"
#include "tgt_mem.h"
#include "kernel/kernel_assert.h"
#include "kernel/console/print_out.h"
#include "kernel/memory/dlmalloc/malloc.h"

/*
 * this structure is used to manage the blocks
 * stored on the heap
 */
typedef struct mem_pool_info_t
{
	/* dlmalloc memory space */
	mspace space;
	/* total heap in bytes */
	uint32_t pool_alloc_size;
} mem_pool_info_internal_t;

bool_t 	mem_init_memory_pool(
		const uint32_t base_addr,
		const uint32_t pool_size,
		mem_pool_info_t ** const pool)
{
	mem_pool_info_t * pool_info = NULL;

#if defined (MEMORY_DEBUGGING)
	debug_print("mem: creating new pool at 0x%x for %d bytes\n", base_addr, pool_size);
#endif

	/*
	 * put the first info block after the heap at the beginning
	 * of the next page. this gives access to the full heap
	 */
	pool_info = (mem_pool_info_t*)(base_addr);

	pool_info->pool_alloc_size = pool_size;
	pool_info->space = create_mspace_with_base(pool_info + sizeof(mem_pool_info_t), pool_size - sizeof(mem_pool_info_t), 0);

	if (pool)
	{
		*pool = pool_info;
	}

#if defined (MEMORY_DEBUGGING)
	debug_print("mem: new pool is at %x\n", pool_info);
#endif

	return true;
}

bool_t	mem_init_process_memory(
		mem_pool_info_t * const pool,
		mem_pool_info_t ** const proc_memory_block,
		const uint32_t size)
{
	bool_t ret = false;

#if defined (MEMORY_DEBUGGING)
	debug_print("mem: initalising process pool %x size %x\n", pool, size);
#endif

	/* allocate that from RAM */
	const uint32_t proc_memory_pool = (uint32_t)mem_alloc_aligned(
			pool,
			size,
			MMU_PAGE_SIZE);

#if defined (MEMORY_DEBUGGING)
	debug_print("mem: initalised process pool %x size %x result %x\n", pool, size, proc_memory_pool);
#endif

	if (proc_memory_pool != 0)
	{
		/* create a table for the given process */
		ret = mem_init_memory_pool(
				proc_memory_pool,
				size,
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
		const uint32_t size)
{
	return mem_alloc_aligned(pool, size, 0);
}

void* mem_alloc_aligned(
		mem_pool_info_t * const pool,
		const uint32_t size,
		const uint32_t alignment)
{
	void* new_base = NULL;

#if defined (MEMORY_DEBUGGING)
	debug_print("mem: looking for %d bytes in pool 0x%x\n", size, pool);
#endif

	kernel_assert("mem: attempt to allocate to a null pool\n", pool != NULL);
	new_base = mspace_memalign(pool->space, alignment, size);

#if defined (MEMORY_DEBUGGING)
	debug_print("mem: base at 0x%x, size 0x%x\n", new_base, size);
#endif

	return new_base;
}

void mem_free(
		mem_pool_info_t * const pool,
		const void * const base)
{
#if defined (MEMORY_DEBUGGING)
	debug_print("mem: free pool 0x%x, base 0x%x\n", pool, base);
#endif
	mspace_free(pool->space, (void*)base);
}

uint32_t mem_get_start_addr(const mem_pool_info_t * const pool)
{
	return (uint32_t)(pool);
}

uint32_t mem_get_alloc_size(const mem_pool_info_t * const pool)
{
	uint32_t size;
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

uint32_t mem_get_allocd_size(const mem_pool_info_t * const pool)
{
	return mspace_footprint(pool->space);
}
