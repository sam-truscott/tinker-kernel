/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_TIMER_H_
#define OBJ_TIMER_H_

#include "sos_api_timer.h"
#include "kernel/objects/object.h"
#include "kernel/process/process.h"

typedef struct __object_timer_t __object_timer_t;

__object_timer_t * __obj_cast_timer(__object_t * const o);

object_number_t __obj_timer_get_oid
	(const __object_timer_t * const o);

error_t __obj_create_timer(
		__process_t * const process,
		object_number_t * objectno,
		const __priority_t priority,
		const uint32_t seconds,
		const uint32_t nanoseconds,
		sos_timer_callback_t * const callback,
		const void* parameter);

error_t __obj_cancel_timer(__object_timer_t * const timer);

error_t __obj_delete_timer(__object_timer_t * const timer);

#endif /* OBJ_TIMER_H_ */
