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

#include "mem_pool.h"

/**
 * Initialise memory services
 */
bool_t __mem_initialise(
		const uint32_t start,
		const uint32_t end);

/**
 * Get the default memory pool
 * @return The default memory pool
 */
__mem_pool_info_t * __mem_get_default_pool(void);

#endif /* MEMORY_MANAGER_H_ */
