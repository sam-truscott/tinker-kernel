/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include "tgt_types.h"
#include "tinker_api_types.h"

typedef enum object_type
{
	UNKNOWN_OBJ = 0,
	OBJECT = 1,
	PROCESS_OBJ = 2,
	THREAD_OBJ = 3,
	PIPE_OBJ = 4,
	SEMAPHORE_OBJ = 5,
	SHARED_MEMORY_OBJ = 6,
	TIMER_OBJ = 7
} object_type_t;

typedef struct object_t object_t;

void obj_initialise_object(
		object_t * const o,
		const object_number_t id,
		const object_type_t type);

object_type_t obj_get_type(const object_t * const o);

uint32_t obj_get_number(const object_t * const o);

#endif /* OBJECT_H_ */
