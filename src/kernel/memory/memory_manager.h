/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include "kernel/kernel_types.h"

/**
 * Initialise memory services
 */
bool __mem_initialise(
		const uint32_t start,
		const uint32_t end);

/**
 * initialise a heap at the given base address and size,
 * the heap_info_type is put at the end of the heap and
 * also returned for information (unless NULL)
 */
bool 	__mem_init_memory_pool(
		const uint32_t base_addr,
		const uint32_t pool_size,
		__mem_pool_info_t **pool);

/**
 * Get the default memory pool
 * @return The default memory pool
 */
__mem_pool_info_t * __mem_get_default_pool(void);

/**
 * initialise a process with a memory pool and a stack
 * for the initialise thread
 */
bool	__mem_init_process_memory(
		__mem_pool_info_t ** proc_memory_block,
		const uint32_t size);

/**
 * dynamically allocate memory from the heap
 */
void *	__mem_alloc(
		__mem_pool_info_t * pool,
		const uint32_t size);

/**
 * dynamically allocate memory from the heap
 */
void *	__mem_alloc_aligned(
		__mem_pool_info_t * pool,
		const uint32_t size,
		const uint32_t alignment);

/**
 * dynamically deallocate memory from the heap
 */
void	__mem_free(
		__mem_pool_info_t * pool,
		void * base);

#endif /* MEMORY_MANAGER_H_ */
