/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_timer.h"
#include "kernel/time/time.h"

typedef struct __object_timer_t
{
	__time_t timeout;
} __object_timer_internal_t;
