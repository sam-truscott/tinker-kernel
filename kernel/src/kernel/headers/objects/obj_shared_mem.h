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
#include "process/process.h"
#include "objects/object.h"
#include "objects/object_table.h"
#include "objects/obj_thread.h"

typedef struct object_shm_t object_shm_t;

object_shm_t * obj_cast_shm(void * const o);

object_number_t obj_shm_get_oid
	(const object_shm_t * const o);

return_t obj_create_shm(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address);

return_t obj_open_shm(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address);

return_t obj_delete_shm(
		object_shm_t * const shm);

#endif /* OBJ_SHARED_MEM_H_ */
