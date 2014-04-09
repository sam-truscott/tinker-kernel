/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include "arch/tgt_types.h"
#include "tinker_api_types.h"

typedef enum __object_type
{
	UNKNOWN_OBJ = 0,
	OBJECT = 1,
	PROCESS_OBJ = 2,
	THREAD_OBJ = 3,
	PIPE_OBJ = 4,
	SEMAPHORE_OBJ = 5,
	SHARED_MEMORY_OBJ = 6,
	TIMER_OBJ = 7
} __object_type_t;

typedef struct __object_t __object_t;

void __obj_initialise_object(
		__object_t * const o,
		const object_number_t id,
		const __object_type_t type);

__object_type_t __obj_get_type(const __object_t * const o);

uint32_t __obj_get_number(const __object_t * const o);

#endif /* OBJECT_H_ */
