/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_THREAD_H_
#define OBJ_THREAD_H_

#include "api/sos_api_time.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"
#include "kernel/process/thread.h"

typedef struct __object_thread_t __object_thread_t;

error_t __obj_create_thread(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		const uint32_t thread_id,
		__thread_t * const thread,
		object_number_t * const object_no);

void __obj_delete_thread(__object_thread_t * const thread);

__object_thread_t * __obj_cast_thread(__object_t * o);

object_number_t __obj_thread_get_oid(const __object_thread_t * const o);

error_t __obj_exit_thread(
		__object_thread_t * const o);

error_t __obj_set_thread_waiting(
		__object_thread_t * const o,
		const __object_t * const waiting_on);

bool_t __obj_thread_is_waiting_on(
		const __object_thread_t * const o,
		const __object_t * const waiting_on);

error_t __obj_set_thread_ready(
		__object_thread_t * const o);

error_t __obj_get_thread_state(
		__object_thread_t * const o,
		__thread_state_t * const state);

error_t __obj_get_thread_priority(
		__object_thread_t * const o,
		uint8_t * const priority);

__priority_t __obj_get_thread_priority_ex(__object_thread_t * const o);

error_t __obj_set_thread_priority(
		__object_thread_t * const o,
		const uint8_t priority);

error_t __obj_reset_thread_original_priority(
		__object_thread_t * const o);

error_t __obj_set_thread_original_priority(
		__object_thread_t * const o);

__priority_t __obj_get_thread_original_priority_ex(__object_thread_t * const o);

__thread_t * __obj_get_thread(const __object_thread_t * const o);

error_t __obj_thread_sleep(__object_thread_t * const o, const sos_time_t * const duration);

#endif /* OBJ_THREAD_H_ */
