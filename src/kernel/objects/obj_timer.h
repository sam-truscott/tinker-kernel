/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_TIMER_H_
#define OBJ_TIMER_H_

#include "tinker_api_timer.h"
#include "kernel/objects/object.h"
#include "kernel/process/process.h"
#include "kernel/scheduler/scheduler.h"

typedef struct object_timer_t object_timer_t;

object_timer_t * obj_cast_timer(object_t * const o);

object_number_t obj_timer_get_oid
	(const object_timer_t * const o);

error_t obj_create_timer(
		scheduler_t * const scheduler,
		process_t * const process,
		object_number_t * objectno,
		const priority_t priority,
		const uint32_t seconds,
		const uint32_t nanoseconds,
		tinker_timer_callback_t * const callback,
		const void* parameter);

error_t obj_cancel_timer(object_timer_t * const timer);

error_t obj_delete_timer(object_timer_t * const timer);

#endif /* OBJ_TIMER_H_ */
