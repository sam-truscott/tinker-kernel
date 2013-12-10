/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_KERNEL_INTERFACE_H_
#define SOS_API_KERNEL_INTERFACE_H_

#include "sos_api.h"

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
	syscall_open_semaphore,
	syscall_get_semaphore,
	syscall_release_semaphore,

	syscall_create_pipe,
	syscall_delete_pipe,
	syscall_open_pipe,
	syscall_close_pipe,
	syscall_send_message,
	syscall_receive_message,
	syscall_received_message,

	syscall_create_shm,
	syscall_open_shm,
	syscall_destroy_shm,

	syscall_debug,

	syscall_load_thread

} __syscall_function_t;

uint32_t SOS_API_CALL_7(
		const __syscall_function_t api,
		uint32_t param_1,
		uint32_t param_2,
		uint32_t param_3,
		uint32_t param_4,
		uint32_t param_5,
		uint32_t param_6,
		uint32_t param_7) SOS_API_SUFFIX;

#define SOS_API_CALL_0(api) \
	SOS_API_CALL_7( \
		api,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS_API_CALL_1(api, param_1) \
	SOS_API_CALL_7( \
		api,\
		param_1,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS_API_CALL_2(api, param_1, param_2) \
	SOS_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS_API_CALL_3(api, param_1, param_2, param_3) \
	SOS_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS_API_CALL_4(api, param_1, param_2, param_3, param_4) \
	SOS_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS_API_CALL_5(api, param_1, param_2, param_3, param_4, param_5) \
	SOS_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		param_5,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define SOS_API_CALL_6(api, param_1, param_2, param_3, param_4, param_5, param_6) \
	SOS_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		param_5,\
		param_6,\
		UNUSED_PARAM)

#endif /* SOS_API_KERNEL_INTERFACE_H_ */
