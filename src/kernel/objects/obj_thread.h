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
		__mem_pool_info_t * pool,
		__object_table_t * table,
	 	const uint32_t process_id,
		const uint32_t thread_id,
		__thread_t * thread,
		__object_t ** object);

error_t __obj_pause_thread(__object_t * o);

error_t __obj_resume_thread(__object_t * o);

error_t __obj_terminate_thread(__object_t * o);

error_t __obj_exit_thread(__object_t * o);

error_t __obj_set_thread_waiting(__object_t * o, __object_t * waiting_on);

error_t __obj_set_thread_ready(__object_t * o);

error_t __obj_get_thread_state(__object_t * o, __thread_state_t * state);

error_t __obj_get_thread_priority(__object_t * o, uint8_t * priority);

error_t __obj_set_thread_priority(__object_t * o, uint8_t priority);

error_t __obj_reset_thread_original_priority(__object_t * o);

error_t __obj_set_thread_original_priority(__object_t * o);

#endif /* OBJ_THREAD_H_ */
