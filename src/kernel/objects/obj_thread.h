/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
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
