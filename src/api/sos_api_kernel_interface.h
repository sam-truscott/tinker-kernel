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
	SYSCALL_UNKNOWN,

	SYSCALL_CREATE_PROCESS,

	SYSCALL_CREATE_THREAD,
	SYSCALL_THREAD_PRIORITY,
	SYSCALL_THREAD_OBJECT,
	SYSCALL_EXIT_THREAD,

	SYSCALL_CREATE_SEMAPHORE,
	SYSCALL_OPEN_SEMAPHORE,
	SYSCALL_GET_SEMAPHORE,
	SYSCALL_RELEASE_SEMAPHORE,
	SYSCALL_CLOSE_SEMAPHORE,

	SYSCALL_CREATE_PIPE,
	SYSCALL_DELETE_PIPE,
	SYSCALL_OPEN_PIPE,
	SYSCALL_CLOSE_PIPE,
	SYSCALL_SEND_MESSAGE,
	SYSCALL_RECEIVE_MESSAGE,
	SYSCALL_RECEIVED_MESSAGE,

	SYSCALL_CREATE_SHM,
	SYSCALL_OPEN_SHM,
	SYSCALL_DESTROY_SHM,

	SYSCALL_CREATE_TIMER,
	SYSCALL_CANCEL_TIMER,
	SYSCALL_DELETE_TIMER,

	SYSCALL_DEBUG,

	SYSCALL_LOAD_THREAD

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
