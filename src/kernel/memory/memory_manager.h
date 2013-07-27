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
#include "mem_pool.h"

/**
 * Initialise memory services
 */
bool __mem_initialise(
		const uint32_t start,
		const uint32_t end);

#endif /* MEMORY_MANAGER_H_ */
