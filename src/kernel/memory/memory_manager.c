/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "memory_manager.h"

#include "arch/tgt_types.h"
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
