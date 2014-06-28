/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TIME_H_
#define TIME_H_

#include "arch/tgt_types.h"

typedef uint64_t (clock_get_nanoseconds_time)(void);

typedef struct
{
	clock_get_nanoseconds_time * 		get_time;
} clock_device_t;

#endif /* TIME_H_ */
