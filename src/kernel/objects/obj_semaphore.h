/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_SEMAPHORE_H_
#define OBJ_SEMAPHORE_H_

#include "../kernel_types.h"

error_t __obj_initialise_semaphore(
		__object_t * object,
		const uint32_t initial_count);

error_t __obj_create_semaphore(
		__mem_pool_info_t * pool,
		__object_table_t * table,
		__object_t ** object,
		const uint32_t initial_count);

error_t __obj_get_semaphore(
		__object_t * thread,
		__object_t * semaphore);

error_t __obj_release_semaphore(
		__object_t * thread,
		__object_t * semaphore);

#endif /* OBJ_SEMAPHORE_H_ */
