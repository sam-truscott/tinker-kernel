/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_TYPES_H_
#define TINKER_API_TYPES_H_

#if defined(BUILDING_KERNEL)
#define TINKER_API_SUFFIX __attribute__((section(".api")))
#else
#define TINKER_API_SUFFIX
#endif

#include "arch/tgt_types.h"

typedef enum thread_state
{
	THREAD_NOT_CREATED = 1,
	THREAD_IDLE = 2,
	THREAD_READY = 3,
	THREAD_RUNNING = 4,
	THREAD_SYSTEM = 5,
	THREAD_PAUSED = 6,
	THREAD_WAITING = 7,
	THREAD_TERMINATED = 8,
	THREAD_DEAD = 9
} thread_state_t;

#define THREAD_FLAG_NONE 0x0
#define THREAD_FLAG_FP 0x1
#define THREAD_FLAG_TIMER 0x2

typedef uint16_t object_number_t;

#define INVALID_OBJECT_ID -1

#endif /* TINKER_API_TYPES_H_ */
