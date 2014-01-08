/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TIME_H_
#define TIME_H_

#include "arch/tgt_types.h"

#define ONE_SECOND_AS_NANOSECONDS	1000000000
#define ONE_MS_AS_NANOSECONDS 		1000000
#define ONE_US_AS_NANOSECONDS 		1000

typedef struct
{
	int32_t seconds;
	int32_t nanoseconds;
} __time_t;

#define __ZERO_TIME ((__time_t){0,0})

typedef uint64_t (__clock_get_nanoseconds_time)(void);

typedef struct
{
	__clock_get_nanoseconds_time * 		get_time;
} __clock_device_t;

#endif /* TIME_H_ */
