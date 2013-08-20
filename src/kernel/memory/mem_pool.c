/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "mem_pool.h"

#include "sos_api_types.h"
#include "tgt_mem.h"
#include "kernel/kernel_assert.h"
#include "kernel/console/print_out.h"
#include "kernel/memory/dlmalloc/malloc.h"

/*
 * this structure is used to manage the blocks
 * stored on the heap
 */
typedef struct __mem_pool_info_t
{
	/* dlmalloc memory space */
	mspace space;
	/* total heap in bytes */
	uint32_t pool_alloc_size;
} __mem_pool_info_internal_t;

bool_t 	__mem_init_memory_pool(
		const uint32_t base_addr,
		const uint32_t pool_size,
		__mem_pool_info_t ** const pool)
{
	__mem_pool_info_t * pool_info = NULL;

#if defined (__MEMORY_DEBUGGING)
	__debug_print("mem: creating new pool at %x for %d bytes\n", base_addr, pool_size);
#endif

	/*
	 * put the first info block after the heap at the beginning
	 * of the next page. this gives access to the full heap
	 */
	pool_info = (__mem_pool_info_t*)(base_addr);

	pool_info->pool_alloc_size = pool_size;
	pool_info->space = create_mspace_with_base(pool_info + sizeof(__mem_pool_info_t), pool_size - sizeof(__mem_pool_info_t), 0);

	if (pool)
	{
		*pool = pool_info;
	}

#if defined (__MEMORY_DEBUGGING)
	__debug_print("mem: new pool is at %x\n", pool_info);
#endif

	return true;
}

bool_t	__mem_init_process_memory(
		__mem_pool_info_t * const pool,
		__mem_pool_info_t ** const proc_memory_block,
		const uint32_t size)
{
	bool_t ret = false;

	/* allocate that from RAM */
	uint32_t proc_memory_pool = (uint32_t)__mem_alloc_aligned(
			pool,
			size,
			MMU_PAGE_SIZE);

	if ( proc_memory_pool != 0 )
	{
		/* create a table for the given process */
		ret = __mem_init_memory_pool(
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

void *	__mem_alloc(
		__mem_pool_info_t * const pool,
		const uint32_t size)
{
	return __mem_alloc_aligned(pool, size, 0);
}

void* __mem_alloc_aligned(
		__mem_pool_info_t * const pool,
		const uint32_t size,
		const uint32_t alignment)
{
	void* new_base = NULL;

#if defined (__MEMORY_DEBUGGING)
	__debug_print("mem: looking for %d bytes in pool %x\n", size, pool);
#endif

	if ( pool != NULL )
	{
		new_base = mspace_memalign(pool->space, alignment, size);
	}
	else
	{
		__kernel_assert("mem: attempt to allocate to a null pool\n", false);
	}

#if defined (__MEMORY_DEBUGGING)
	__debug_print("mem: base at %x, size %x\n", new_base, size);
#endif

	return new_base;
}

void __mem_free(
		__mem_pool_info_t * const pool,
		const void * const base)
{
	mspace_free(pool->space, (void*)base);
}

uint32_t __mem_get_start_addr(const __mem_pool_info_t * const pool)
{
	return (uint32_t)(pool + sizeof(__mem_pool_info_t));
}

uint32_t __mem_get_alloc_size(const __mem_pool_info_t * const pool)
{
	return pool->pool_alloc_size;
}
