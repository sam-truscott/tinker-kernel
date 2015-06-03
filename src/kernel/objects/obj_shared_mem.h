/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_SHARED_MEM_H_
#define OBJ_SHARED_MEM_H_

#include "registry.h"
#include "kernel/process/process.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"

typedef struct object_shm_t object_shm_t;

object_shm_t * obj_cast_shm(object_t * const o);

object_number_t obj_shm_get_oid
	(const object_shm_t * const o);

error_t obj_create_shm(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address);

error_t obj_open_shm(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address);

error_t obj_delete_shm(
		object_shm_t * const shm);

#endif /* OBJ_SHARED_MEM_H_ */
