/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TIMER_UTILITIES_H_
#define TIMER_UTILITIES_H_

#include "../kernel_types.h"

__time_t __time_add(__time_t l, __time_t r);

__time_t __time_sub(__time_t l, __time_t r);

bool __time_lt(__time_t l, __time_t r);

bool __time_gt(__time_t l, __time_t r);

bool __time_eq(__time_t l, __time_t r);

__time_t __time_seconds(uint32_t seconds);

__time_t __time_milliseconds(uint32_t milliseconds);

__time_t __time_microseconds(uint32_t microseconds);

__time_t __time_nanoseconds(uint64_t nanoseconds);

#endif /* TIMER_UTILITIES_H_ */
