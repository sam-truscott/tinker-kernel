/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "memory/memory_manager.h"

#include "tgt_types.h"
#include "console/print_out.h"
#include "kernel_assert.h"

/**
 * the primary memory pool i.e. RAM
 */
static mem_pool_info_t *mem_memory_pool = NULL;

mem_pool_info_t * mem_get_default_pool(void)
{
	return mem_memory_pool;
}

bool_t mem_initialise(
		const uint32_t start,
		const uint32_t end)
{
	bool_t ret =  mem_init_memory_pool(
		start,
		end - start,
		(&mem_memory_pool));

	return ret;
}
