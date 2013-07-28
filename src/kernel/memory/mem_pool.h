/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef MEM_POOL_H_
#define MEM_POOL_H_

#include "arch/tgt_types.h"

typedef struct __mem_pool_info_t __mem_pool_info_t;

#define MMU_NO_ACCESS	0
#define MMU_R_ACCESS	1
#define MMU_W_ACCESS	2
#define MMU_E_ACCESS	4

typedef uint32_t seg_id_t;

/**
 * initialise a heap at the given base address and size,
 * the heap_info_type is put at the end of the heap and
 * also returned for information (unless NULL)
 */
bool 	__mem_init_memory_pool(
		const uint32_t base_addr,
		const uint32_t pool_size,
		__mem_pool_info_t ** const pool);

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
		__mem_pool_info_t * const pool,
		__mem_pool_info_t ** const proc_memory_block,
		const uint32_t size);

/**
 * dynamically allocate memory from the heap
 */
void *	__mem_alloc(
		__mem_pool_info_t * const pool,
		const uint32_t size);

/**
 * dynamically allocate memory from the heap
 */
void *	__mem_alloc_aligned(
		__mem_pool_info_t * const pool,
		const uint32_t size,
		const uint32_t alignment);

/**
 * dynamically deallocate memory from the heap
 */
void	__mem_free(
		__mem_pool_info_t * const pool,
		const void * const base);

uint32_t __mem_get_start_addr(const __mem_pool_info_t * const pool);
uint32_t __mem_get_alloc_size(const __mem_pool_info_t * const pool);

#endif /* MEM_POOL_H_ */
