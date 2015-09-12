/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "test_memory.h"

#include "kernel/kernel_assert.h"
#include "kernel/memory/memory_manager.h"

void test_memory(void)
{
	mem_pool_info_t * const pool = mem_get_default_pool();
	kernel_assert("collection present", pool != NULL);
	const uint32_t allocated = mem_get_allocd_size(pool);

	void * base = mem_realloc(pool, 0, 128);
	mem_free(pool, base);

	kernel_assert("collection memory leak in pool", allocated == mem_get_allocd_size(pool));

	base = mem_realloc(pool, 0, 128);
	base = mem_realloc(pool, base, 256);
	mem_free(pool, base);

	kernel_assert("collection memory leak in pool", allocated == mem_get_allocd_size(pool));
}


