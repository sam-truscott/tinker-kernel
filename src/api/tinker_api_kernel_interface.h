/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_KERNEL_INTERFACE_H_
#define TINKER_API_KERNEL_INTERFACE_H_

#include "tinker_api.h"

#define UNUSED_PARAM (uint32_t)0

#define SYSCALL_TEST_1 0x11111111
#define SYSCALL_TEST_2 0x22222222
#define SYSCALL_TEST_3 0x33333333
#define SYSCALL_TEST_4 0x44444444
#define SYSCALL_TEST_5 0x55555555
#define SYSCALL_TEST_6 0x66666666
#define SYSCALL_TEST_7 0x77777777

typedef enum syscall_function
{
	SYSCALL_UNKNOWN,

	SYSCALL_TEST,

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

	SYSCALL_GET_TIME,
	SYSCALL_SLEEP,

	SYSCALL_CREATE_TIMER,
	SYSCALL_CANCEL_TIMER,
	SYSCALL_DELETE_TIMER,

	SYSCALL_DEBUG,

	SYSCALL_LOAD_THREAD

} syscall_function_t;

uint32_t TINKER_API_CALL_7(
		const syscall_function_t api,
		uint32_t param_1,
		uint32_t param_2,
		uint32_t param_3,
		uint32_t param_4,
		uint32_t param_5,
		uint32_t param_6,
		uint32_t param_7) TINKER_API_SUFFIX;

#define TINKER_API_CALL_0(api) \
	TINKER_API_CALL_7( \
		api,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define TINKER_API_CALL_1(api, param_1) \
	TINKER_API_CALL_7( \
		api,\
		param_1,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define TINKER_API_CALL_2(api, param_1, param_2) \
	TINKER_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define TINKER_API_CALL_3(api, param_1, param_2, param_3) \
	TINKER_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define TINKER_API_CALL_4(api, param_1, param_2, param_3, param_4) \
	TINKER_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		UNUSED_PARAM,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define TINKER_API_CALL_5(api, param_1, param_2, param_3, param_4, param_5) \
	TINKER_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		param_5,\
		UNUSED_PARAM,\
		UNUSED_PARAM)

#define TINKER_API_CALL_6(api, param_1, param_2, param_3, param_4, param_5, param_6) \
	TINKER_API_CALL_7( \
		api,\
		param_1,\
		param_2,\
		param_3,\
		param_4,\
		param_5,\
		param_6,\
		UNUSED_PARAM)

#endif /* TINKER_API_KERNEL_INTERFACE_H_ */
