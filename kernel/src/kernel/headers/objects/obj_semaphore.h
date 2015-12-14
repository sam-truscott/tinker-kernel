/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_SEMAPHORE_H_
#define OBJ_SEMAPHORE_H_

#include "registry.h"
#include "process/process.h"
#include "objects/object.h"
#include "objects/obj_thread.h"

typedef struct object_sema_t object_sema_t;

object_sema_t * obj_cast_semaphore(object_t * o);

object_number_t obj_semaphore_get_oid
	(const object_sema_t * const o);

error_t obj_create_semaphore(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t initial_count);

error_t obj_open_semaphore(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name);

error_t object_delete_semaphore(
		object_sema_t * const semaphore);

error_t obj_get_semaphore(
		object_thread_t * const thread,
		object_sema_t * const semaphore);

error_t obj_release_semaphore(
		object_thread_t * const thread,
		object_sema_t * const semaphore);

uint32_t obj_get_sema_count(const object_sema_t * const sema);

uint32_t obj_get_sema_alloc(const object_sema_t * const sema);

priority_t obj_get_sema_highest_priority(const object_sema_t * const sema);

#endif /* OBJ_SEMAPHORE_H_ */
