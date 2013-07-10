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
#include "kernel/objects/object.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/objects/object_table.h"

typedef struct __object_sema_t __object_sema_t;

error_t __obj_create_semaphore(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		__object_t ** object,
		const uint32_t initial_count);

error_t __obj_get_semaphore(
		__object_thread_t * const thread,
		__object_sema_t * const semaphore);

error_t __obj_release_semaphore(
		__object_thread_t * const thread,
		__object_sema_t * const semaphore);

#endif /* OBJ_SEMAPHORE_H_ */
