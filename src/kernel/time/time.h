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

typedef uint64_t (clock_get_nanoseconds_time)(void * user_data);

typedef struct
{
	clock_get_nanoseconds_time * 		get_time;
	void* user_data;
} clock_device_t;

#endif /* TIME_H_ */
