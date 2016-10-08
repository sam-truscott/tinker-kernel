/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef MEM_POOL_H_
#define MEM_POOL_H_

#include "tgt_types.h"

typedef struct mem_pool_info_t mem_pool_info_t;

typedef uint32_t seg_id_t;

/**
 * initialise a heap at the given base address and size,
 * the heap_info_type is put at the end of the heap and
 * also returned for information (unless NULL)
 */
bool_t 	mem_init_memory_pool(
		const mem_t base_addr,
		const mem_t pool_size,
		mem_pool_info_t ** const pool);

/**
 * initialise a process with a memory pool and a stack
 * for the initialise thread
 */
bool_t	mem_init_process_memory(
		mem_pool_info_t * const pool,
		mem_pool_info_t ** const proc_memory_block,
		const uint32_t size);

/**
 * dynamically allocate memory from the heap
 */
void *	mem_alloc(
		mem_pool_info_t * const pool,
		const mem_t size);

/**
 * dynamically re-allocate memory from the heap
 */
void *  mem_realloc(
		mem_pool_info_t * const pool,
		void * mem,
		const mem_t size);

/**
 * dynamically allocate memory from the heap
 */
void *	mem_alloc_aligned(
		mem_pool_info_t * const pool,
		const mem_t size,
		const mem_t alignment);

/**
 * dynamically deallocate memory from the heap
 */
void	mem_free(
		mem_pool_info_t * const pool,
		const void * const base);

mem_t mem_get_start_addr(const mem_pool_info_t * const pool);
mem_t mem_get_alloc_size(const mem_pool_info_t * const pool);
mem_t mem_get_allocd_size(const mem_pool_info_t * const pool);

#endif /* MEM_POOL_H_ */
