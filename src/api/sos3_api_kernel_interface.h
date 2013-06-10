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

#ifndef SOS2_API_KERNEL_INTERFACE_H_
#define SOS2_API_KERNEL_INTERFACE_H_

#include "sos3_api.h"

#define UNUSED_PARAM (uint32_t)0

typedef enum __syscall_function
{
	syscall_unknown,

	syscall_create_process,

	syscall_create_thread,
	syscall_pause_thread,
	syscall_resume_thread,
	syscall_yield_thread,
	syscall_thread_stack_size,
	syscall_thread_priority,
	syscall_thread_object,
	syscall_exit_thread,

	syscall_create_semaphore,
	syscall_get_semaphore,
	syscall_release_semaphore,

	syscall_malloc,
	syscall_mfree,
	syscall_memset,

	syscall_load_thread

} __syscall_function_t;

/*
#define SOS2_API_CALL_7( \
		api, \
		param_1, \
		param_2, \
		param_3, \
		param_4, \
		param_5, \
		param_6, \
		param_7) \
		__asm__("sc"); \
*/

uint32_t SOS2_API_CALL_7(
		const __syscall_function_t api,
		uint32_t param_1,
		uint32_t param_2,
		uint32_t param_3,
		uint32_t param_4,
		uint32_t param_5,
		uint32_t param_6,
		uint32_t param_7) SOS3_API_SUFFIX;

#define SOS2_API_CALL_0(api) \
	SOS2_API_CALL_7( \
		api,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS2_API_CALL_1(api, param_1) \
	SOS2_API_CALL_7( \
		api,\
		param_1,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS2_API_CALL_2(api, param_1, param_2) \
	SOS2_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS2_API_CALL_3(api, param_1, param_2, param_3) \
	SOS2_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS2_API_CALL_4(api, param_1, param_2, param_3, param_4) \
	SOS2_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS2_API_CALL_5(api, param_1, param_2, param_3, param_4, param_5) \
	SOS2_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		param_5,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS2_API_CALL_6(api, param_1, param_2, param_3, param_4, param_5, param_6) \
	SOS2_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		param_5,\
		param_6,\
		UNUSED_PARAM)

#endif /* SOS2_API_KERNEL_INTERFACE_H_ */
