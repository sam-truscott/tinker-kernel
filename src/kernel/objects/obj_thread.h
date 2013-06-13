/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_THREAD_H_
#define OBJ_THREAD_H_

#include "../kernel_types.h"

error_t __obj_create_thread(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
	 	const uint32_t process_id,
		const uint32_t thread_id,
		__thread_t * const thread,
		__object_t ** object);

error_t __obj_pause_thread(__object_t * const o);

error_t __obj_resume_thread(__object_t * const o);

error_t __obj_terminate_thread(__object_t * const o);

error_t __obj_exit_thread(__object_t * const o);

error_t __obj_set_thread_waiting(
		__object_t * const o,
		const __object_t * const waiting_on);

error_t __obj_set_thread_ready(__object_t * const o);

error_t __obj_get_thread_state(
		__object_t * const o,
		__thread_state_t * const state);

error_t __obj_get_thread_priority(
		__object_t * const o,
		uint8_t * const priority);

error_t __obj_set_thread_priority(
		__object_t * const o,
		const uint8_t priority);

error_t __obj_reset_thread_original_priority(__object_t * const o);

error_t __obj_set_thread_original_priority(__object_t * const o);

#endif /* OBJ_THREAD_H_ */
