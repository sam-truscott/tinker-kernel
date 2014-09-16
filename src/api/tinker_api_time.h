/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_TIME_H_
#define TINKER_API_TIME_H_

#include "arch/tgt_types.h"
#include "api/tinker_api_types.h"
#include "api/tinker_api_errors.h"

typedef struct
{
	int32_t seconds;
	int32_t nanoseconds;
} tinker_time_t;

#define TINKER_ZERO_TIME ((tinker_time_t){0,0})
#define ONE_SECOND_AS_NANOSECONDS	1000000000
#define ONE_MS_AS_NANOSECONDS 		1000000
#define ONE_US_AS_NANOSECONDS 		1000

void tinker_time_add(
		const tinker_time_t * const l,
		const tinker_time_t * const r,
		tinker_time_t * const a) TINKER_API_SUFFIX;

void tinker_time_sub(
		const tinker_time_t * const l,
		const tinker_time_t * const r,
		tinker_time_t * const a) TINKER_API_SUFFIX;

bool_t tinker_time_lt(const tinker_time_t * const l, const tinker_time_t * const r) TINKER_API_SUFFIX;

bool_t tinker_time_gt(const tinker_time_t * const l, const tinker_time_t * const r) TINKER_API_SUFFIX;

bool_t tinker_time_eq(const tinker_time_t * const l, const tinker_time_t * const r) TINKER_API_SUFFIX;

void tinker_time_seconds(const uint32_t seconds, tinker_time_t * const a) TINKER_API_SUFFIX;

void tinker_time_milliseconds(const uint32_t milliseconds, tinker_time_t * const a) TINKER_API_SUFFIX;

void tinker_time_microseconds(const uint32_t microseconds, tinker_time_t * const a) TINKER_API_SUFFIX;

void tinker_time_nanoseconds(const uint64_t nanoseconds, tinker_time_t * const a) TINKER_API_SUFFIX;

uint32_t tinker_timer_get_microseconds(const tinker_time_t * const t) TINKER_API_SUFFIX;

error_t tinker_get_time(tinker_time_t * const time) TINKER_API_SUFFIX;

error_t tinker_sleep(const tinker_time_t * const time) TINKER_API_SUFFIX;

#endif /* TINKER_API_TIME_H_ */
