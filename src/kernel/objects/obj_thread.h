/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_THREAD_H_
#define OBJ_THREAD_H_

#include "api/tinker_api_time.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"
#include "kernel/process/thread.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/time/alarm_manager.h"

typedef struct object_thread_t object_thread_t;

error_t obj_create_thread(
		mem_pool_info_t * const pool,
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager,
		object_table_t * const table,
		const uint32_t thread_id,
		thread_t * const thread,
		object_number_t * const object_no);

void obj_delete_thread(object_thread_t * const thread);

object_thread_t * obj_cast_thread(object_t * o);

object_number_t obj_thread_get_oid(const object_thread_t * const o);

error_t obj_exit_thread(
		object_thread_t * const o);

error_t obj_set_thread_waiting(
		object_thread_t * const o,
		const object_t * const waiting_on);

bool_t obj_thread_is_waiting_on(
		const object_thread_t * const o,
		const object_t * const waiting_on);

error_t obj_set_thread_ready(
		object_thread_t * const o);

error_t obj_get_thread_state(
		object_thread_t * const o,
		thread_state_t * const state);

error_t obj_get_thread_priority(
		object_thread_t * const o,
		uint8_t * const priority);

priority_t obj_get_thread_priority_ex(object_thread_t * const o);

error_t obj_set_thread_priority(
		object_thread_t * const o,
		const uint8_t priority);

error_t obj_reset_thread_original_priority(
		object_thread_t * const o);

error_t obj_set_thread_original_priority(
		object_thread_t * const o);

priority_t obj_get_thread_original_priority_ex(object_thread_t * const o);

thread_t * obj_get_thread(const object_thread_t * const o);

error_t obj_thread_sleep(object_thread_t * const o, const tinker_time_t * const duration);

#endif /* OBJ_THREAD_H_ */
