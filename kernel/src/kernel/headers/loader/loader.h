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

void load_elf(
		loader_t * const loader,
		const void * const data);

#endif /* LOADER_H_ */
