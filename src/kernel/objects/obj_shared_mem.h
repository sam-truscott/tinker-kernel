/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_SHARED_MEM_H_
#define OBJ_SHARED_MEM_H_

#include "kernel/process/process.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"

typedef struct __object_shm_t __object_shm_t;

__object_shm_t * __obj_cast_shm(__object_t * const o);

object_number_t __obj_shm_get_oid
	(const __object_shm_t * const o);

error_t __obj_create_shm(
		__process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address);

error_t __obj_open_shm(
		__process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t size,
		void ** address);

error_t __obj_delete_shm(
		__object_shm_t * const shm);

#endif /* OBJ_SHARED_MEM_H_ */
