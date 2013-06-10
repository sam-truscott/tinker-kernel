/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_TYPES_H_
#define SOS_API_TYPES_H_

#ifdef __BUILDING_KERNEL
#define SOS_API_SUFFIX __attribute__((section(".user_text")))
#else
#define SOS_API_SUFFIX
#endif

#include "arch/target_types.h"

#define EXTERN extern
#define INLINE inline

#define NULL ((void*)0)

typedef enum
{
	false = 0,
	true = 1
} bool;

typedef enum __thread_state
{
	thread_not_created = 1,
	thread_idle,
	thread_ready,
	thread_running,
	thread_system,
	thread_paused,
	thread_waiting,
	thread_terminated,
} __thread_state_t;

#define THREAD_FLAG_NONE 0x0
#define THREAD_FLAG_FP 0x1

typedef uint16_t object_number_t;

#endif /* SOS_API_TYPES_H_ */
