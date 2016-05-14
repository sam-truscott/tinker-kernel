/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2016] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef LOADER_H_
#define LOADER_H_


#include "process/process_list.h"

typedef struct loader_t loader_t;

loader_t * loader_create(
		mem_pool_info_t * const pool,
		proc_list_t * const list);

return_t load_elf(
		loader_t * const loader,
		const void * const data,
		const char * const image,
		const priority_t priority,
		const uint32_t flags);

#endif /* LOADER_H_ */
