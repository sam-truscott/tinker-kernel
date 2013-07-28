/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include "arch/tgt_types.h"
#include "sos_api_types.h"

typedef enum __object_type
{
	UNKNOWN_OBJ = 0,
	OBJECT = 1,
	PROCESS_OBJ = 2,
	THREAD_OBJ = 3,
	PIPE_OBJ = 4,
	SEMAPHORE_OBJ = 5,
	SHARED_MEMORY_OBJ = 6,
	CLOCK_OBJ = 7,
	TIMER_OBJ = 8
} __object_type_t;

typedef struct __object_t __object_t;

void __obj_initialise_object(
		__object_t * const o,
		const object_number_t id,
		const __object_type_t type);

bool __obj_is_initialised(__object_t * const o);

bool __obj_is_allocated(__object_t * const o);

uint32_t __obj_get_number(__object_t * const o);

__object_type_t __obj_get_type(__object_t * const o);

void __obj_lock(__object_t * const o);

void __obj_release(__object_t * const o);

void __obj_increase_ref_count(__object_t * const o);

void __obj_decrease_ref_count(__object_t * const o);

uint32_t __obj_get_ref_count(const __object_t * const o);

#endif /* OBJECT_H_ */
