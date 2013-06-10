/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "memory_manager.h"

#include "kernel/debug/debug_print.h"
#include "kernel/kernel_assert.h"

/* #define __MEMORY_DEBUGGING 1 */

/*
 * TODO This implementation needs to take account for
 * alignment. i.e. it may be that the user wants to access
 * a 32-bit aligned memory access. should this be part of the
 * user-mode code? doubtful as it won't contain the right information.
 *
 * might have to update the memory management storage blocks to
 * be allowed to start at aligned boundaries but this could get
 * wasteful.
 */

static void __mem_find_free_block(
		__mem_pool_info_t ** pool,
		const uint32_t size,
		const uint32_t alignment,
		__mem_block_t ** ret_block);

static void __mem_allocate_block(
		__mem_pool_info_t * pool,
		const uint32_t size,
		__mem_block_t * block,
		__mem_block_t ** new_block);

static void __mem_deallocate_block(
		__mem_pool_info_t * pool,
		__mem_block_t * block);

static void __mem_join_blocks(
		__mem_pool_info_t * pool,
		__mem_block_t * free_block,
		const __mem_block_t * freed_block);

/**
 * the primary memory pool i.e. RAM
 */
static __mem_pool_info_t *__mem_memory_pool = NULL;

__mem_pool_info_t * __mem_get_default_pool(void)
{
	return __mem_memory_pool;
}

bool __mem_initialise(
		const uint32_t start,
		const uint32_t end)
{
	bool ret =  __mem_init_memory_pool(
		start,
		end - start,
		(&__mem_memory_pool));

	return ret;
}

bool 	__mem_init_memory_pool(
		const uint32_t base_addr,
		const uint32_t pool_size,
		__mem_pool_info_t ** pool)
{
	uint32_t temp = 0;
	uint32_t pool_base = base_addr;
	__mem_pool_info_t * pool_info = NULL;
	__mem_block_t * first_block = NULL;

	/* work out if the base is page aligned */
	temp = pool_base % MMU_PAGE_SIZE;

	if ( temp > 0 )
	{
		pool_base = base_addr + (MMU_PAGE_SIZE - temp);
	}

#if defined (__MEMORY_DEBUGGING )
	__debug_print("mem: creating new pool at %x for %d bytes\n", pool_base, pool_size);
#endif

	/*
	 * put the first info block after the heap at the beginning
	 * of the next page. this gives access to the full heap
	 */
	pool_info =
		(__mem_pool_info_t*)((base_addr + pool_size)
			- (MMU_PAGE_SIZE - sizeof(__mem_pool_info_t)));

	pool_info->start_pool = base_addr;
	pool_info->total_pool_size =
			pool_size - sizeof(__mem_pool_info_t) - sizeof(__mem_block_t);
	pool_info->end_pool
		= pool_info->start_pool + pool_info->total_pool_size;
	pool_info->blocks = 0;
	pool_info->free_pool_bytes = pool_info->total_pool_size;
	pool_info->page_size = MMU_PAGE_SIZE;
	pool_info->total_pages
		= pool_info->total_pool_size / pool_info->page_size;
	pool_info->next_pool_info = NULL;

	first_block
		= (__mem_block_t*)((uint8_t*)pool_info - sizeof(__mem_pool_info_t));
	first_block->addr = base_addr;
	first_block->size = pool_info->total_pool_size;
	first_block->is_free = true;
	pool_info->blocks++;

	pool_info->first_block = first_block;

	if ( pool )
	{
		*pool = pool_info;
	}

#if defined (__MEMORY_DEBUGGING )
	__debug_print("mem: new pool is at %x\n", pool_info);
#endif

	return true;
}

bool	__mem_init_process_memory(
		__mem_pool_info_t ** proc_memory_block,
		const uint32_t size)
{
	bool ret = false;

	/* allocate that from RAM */
	uint32_t proc_memory_pool = (uint32_t)__mem_alloc(
			__mem_memory_pool,
			size);

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
		__mem_pool_info_t * pool,
		const uint32_t size)
{
	return __mem_alloc_aligned(pool, size, 0);
}

void* __mem_alloc_aligned(
		__mem_pool_info_t * pool,
		const uint32_t size,
		const uint32_t alignment)
{
	void* new_base = NULL;
	__mem_block_t * free_block = NULL;

#if defined (__MEMORY_DEBUGGING )
	__debug_print("\nmem: looking for %d bytes in pool %x\n", size, pool);
#endif

	if ( pool != NULL )
	{
		__mem_find_free_block(
				&pool,
				size,
				alignment,
				&free_block);

		if ( free_block != NULL )
		{
			__mem_block_t * new_block;
			new_base = (void*)free_block->addr;

#if defined (__MEMORY_DEBUGGING )
			__debug_print("mem: block found with %d bytes\n", free_block->size);
#endif

			__mem_allocate_block(
					pool,
					size,
					free_block,
					&new_block);

#if defined (__MEMORY_DEBUGGING )
			__debug_print("mem: old block now %d, new block is %d\n",
					free_block->size,
					new_block->size);
#endif
		}
	}
	else
	{
		__kernel_assert("mem: attempt to allocate to a null pool\n", false);
	}

#if defined (__MEMORY_DEBUGGING )
	__debug_print("mem: base at %x, size %x\n\n", new_base, size);
#endif

	return new_base;
}

void __mem_find_free_block(
		__mem_pool_info_t ** pool,
		const uint32_t size,
		const uint32_t alignment,
		__mem_block_t **ret_block)
{
	__mem_pool_info_t * pool_info = NULL;
	bool found = false;

	if ( pool )
	{
		pool_info = *pool;
	}

	while ( pool_info != NULL && found == false )
	{
		__mem_block_t *the_block = pool_info->first_block;

		for ( uint32_t block = 0 ; block < pool_info->blocks ; block++ )
		{
			uint16_t offset = 0;
			if ( alignment > 0 )
			{
				/* increase the address until we find a suitable alignment */
				while (((the_block->addr + offset) % alignment) != 0 )
				{
					offset++;
				}
			}
			if ( the_block->is_free == true &&
					(the_block->size - offset) >= size )
			{
				if ( ret_block )
				{
					*ret_block = the_block;
				}
				if ( offset )
				{
					the_block->addr+=offset;
					the_block->size-=offset;
					the_block->offset = offset;
				}
				*pool = pool_info;
				found = true;
				break;
			}
			the_block--;
		}
		pool_info = pool_info->next_pool_info;
	}
}

void __mem_allocate_block(
		__mem_pool_info_t * pool,
		const uint32_t size,
		__mem_block_t * block,
		__mem_block_t ** new_block)
{
	__mem_block_t * tmp_block = NULL;
	uint32_t offset = 0u;

	/* TODO work out if the offset is large
	 * than the size of a page - if so we
	 * need to create a need page for the next
	 * heap_info_type
	 *
	 * we can use find free block to determine where
	 * one is (pointer) based on the page size and then
	 * setup the heap_info_type and update the next
	 * block on this to point to it ready for the
	 * next allocation
	 */

	if ( pool && block )
	{
		offset = pool->blocks * sizeof(__mem_block_t);

		/* get the new block and update it */
		tmp_block = (__mem_block_t*)((uint8_t*)pool->first_block
						- offset);
		tmp_block->addr = block->addr;
		tmp_block->size = size;
		tmp_block->is_free = false;

		/* update the original block */
		block->addr += size;
		block->size -= size;

#if defined (__MEMORY_DEBUGGING )
		__debug_print("block found; free block now contains %d bytes\n", block->size);
#endif

		pool->blocks++;

		if (new_block)
		{
			*new_block = tmp_block;
		}
	}
}

void __mem_deallocate_block(
		__mem_pool_info_t * pool,
		__mem_block_t * block)
{
	__mem_block_t *next = NULL;
	__mem_block_t *prev = NULL;
	__mem_block_t *curr = NULL;

	if ( pool && block )
	{
		/* mark the block as free */
		block->is_free = true;

		if ( block->offset )
		{
			block->addr -= block->offset;
			block->size += block->offset;
		}

		curr = pool->first_block;
		for ( uint32_t block_no = 0 ; block_no < pool->blocks ; block_no++ )
		{

			/* detect prev block */
			if ( curr->addr + curr->size == block->addr)
			{
				prev = curr;
			}

			/* detect next block */
			if ( block->addr + block->size == curr->addr )
			{
				next = curr;
			}

			/* move on to the next block */
			curr--;
		}

		/* if the next/prev blocks exist and they're
		 * free then we need to join them together.
		 *
		 * the block thats redundant can be removed and
		 * all others blocks after that must be shifted down
		 */
		if ( next != NULL && next->is_free == true )
		{
			__mem_join_blocks(
					pool,
					next,
					block);
		}

		if ( prev != NULL && prev->is_free == true )
		{
			__mem_join_blocks(
					pool,
					prev,
					block);
		}

		/* see if the prev & next block can be put together
		 * this will occur where two free blocks had an allocated
		 * block in between them that's now free
		 */
		if ( prev != NULL && next != NULL)
		{
			if ( prev->addr + prev->size == next->addr)
			{
				__mem_join_blocks(
						pool,
						prev,
						next);
			}
		}
	}
}

void __mem_join_blocks(
		__mem_pool_info_t * pool,
		__mem_block_t * free_block,
		const __mem_block_t * freed_block)
{
	__mem_block_t * block = NULL;
	__mem_block_t * prev_block = NULL;

	if ( pool && free_block && freed_block )
	{
		/* increase the size */
		free_block->size += freed_block->size;

		block = pool->first_block;
		for ( uint32_t block_no = 0 ; block_no < pool->blocks ; block_no++ )
		{
			if ( prev_block != NULL)
			{
				*prev_block = *block;
			}
			else if ( block == freed_block )
			{
				prev_block = block;
			}

			block--;
		}

		/*
		 * If the previous block isn't empty
		 * we'll blank it out to ensure it's no
		 * longer used
		 */
		if ( prev_block != NULL )
		{
			prev_block->addr = 0;
			prev_block->size = 0;
			prev_block->is_free = 0;
		}

		pool->blocks--;
	}
}

void __mem_free(
		__mem_pool_info_t * pool,
		void * base)
{
	__mem_pool_info_t * pool_info = pool;
	bool found = false;

	while ( pool_info != NULL && found == false )
	{
		__mem_block_t *the_block = pool_info->first_block;

		/* iterate over all the blocks and find an
		 * allocated block with the same base address
		 */
		for ( uint32_t block = 0 ; block < pool_info->blocks ; block++ )
		{
			if ( the_block->is_free == false &&
					the_block->addr == (uint32_t)base)
			{
				__mem_deallocate_block(
						pool,
						the_block);
				found = true;
				break;
			}
			the_block--;
		}

		pool_info = pool_info->next_pool_info;
	}
}
