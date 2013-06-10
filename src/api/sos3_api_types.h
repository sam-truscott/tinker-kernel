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

#ifndef SOS2_API_TYPES_H_
#define SOS2_API_TYPES_H_

#ifdef __BUILDING_KERNEL
#define SOS3_API_SUFFIX __attribute__((section(".user_text")))
#else
#define SOS3_API_SUFFIX
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

#endif /* SOS2_API_TYPES_H_ */
