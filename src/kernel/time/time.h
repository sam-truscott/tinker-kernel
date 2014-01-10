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

typedef uint64_t (__clock_get_nanoseconds_time)(void);

typedef struct
{
	__clock_get_nanoseconds_time * 		get_time;
} __clock_device_t;

#endif /* TIME_H_ */
