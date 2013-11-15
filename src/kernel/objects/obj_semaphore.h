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

#include "kernel/process/process.h"
#include "kernel/objects/object.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/objects/object_table.h"

typedef struct __object_sema_t __object_sema_t;

__object_sema_t * __obj_cast_semaphore(__object_t * o);

object_number_t __obj_semaphore_get_oid
	(const __object_sema_t * const o);

error_t __obj_create_semaphore(
		__process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t initial_count);

error_t __obj_open_semaphore(
		__process_t * const process,
		object_number_t * objectno,
		const char * name);

error_t __object_delete_semaphore(
		__object_sema_t * const semaphore);

error_t __obj_get_semaphore(
		__object_thread_t * const thread,
		__object_sema_t * const semaphore);

error_t __obj_release_semaphore(
		__object_thread_t * const thread,
		__object_sema_t * const semaphore);

uint32_t __obj_get_sema_count(const __object_sema_t * const sema);

uint32_t __obj_get_sema_alloc(const __object_sema_t * const sema);

priority_t __obj_get_sema_highest_priority(const __object_sema_t * const sema);

#endif /* OBJ_SEMAPHORE_H_ */
