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

#if defined(__BUILDING_KERNEL)
#define SOS_API_SUFFIX __attribute__((section(".utext")))
#else
#define SOS_API_SUFFIX
#endif

#include "arch/tgt_types.h"

typedef enum __thread_state
{
	THREAD_NOT_CREATED = 1,
	THREAD_IDLE,
	THREADY_READY,
	THREAD_RUNNING,
	THREAD_SYSTEM,
	THREAD_PAUSED,
	THREAD_WAITING,
	THREAD_TERMINATED
} __thread_state_t;

#define THREAD_FLAG_NONE 0x0
#define THREAD_FLAG_FP 0x1

typedef uint16_t object_number_t;

#define INVALID_OBJECT_ID -1

#endif /* SOS_API_TYPES_H_ */
