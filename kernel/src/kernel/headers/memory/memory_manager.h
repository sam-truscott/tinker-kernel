/*
 *
 * TINKER Source Code
 * 
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
bool_t mem_initialise(
		const mem_t start,
		const mem_t end);

/**
 * Get the default memory pool
 * @return The default memory pool
 */
mem_pool_info_t * mem_get_default_pool(void);

#endif /* MEMORY_MANAGER_H_ */
