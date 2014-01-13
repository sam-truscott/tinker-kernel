/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_TIME_H_
#define SOS_API_TIME_H_

#include "arch/tgt_types.h"
#include "api/sos_api_types.h"
#include "api/sos_api_errors.h"

typedef struct
{
	int32_t seconds;
	int32_t nanoseconds;
} sos_time_t;

#define SOS_ZERO_TIME ((sos_time_t){0,0})
#define ONE_SECOND_AS_NANOSECONDS	1000000000
#define ONE_MS_AS_NANOSECONDS 		1000000
#define ONE_US_AS_NANOSECONDS 		1000

sos_time_t sos_time_add(const sos_time_t * const l, const sos_time_t * const r) SOS_API_SUFFIX;

sos_time_t sos_time_sub(const sos_time_t * const l, const sos_time_t * const r) SOS_API_SUFFIX;

bool_t sos_time_lt(const sos_time_t * const l, const sos_time_t * const r) SOS_API_SUFFIX;

bool_t sos_time_gt(const sos_time_t * const l, const sos_time_t * const r) SOS_API_SUFFIX;

bool_t sos_time_eq(const sos_time_t * const l, const sos_time_t * const r) SOS_API_SUFFIX;

sos_time_t sos_time_seconds(const uint32_t seconds) SOS_API_SUFFIX;

sos_time_t sos_time_milliseconds(const uint32_t milliseconds) SOS_API_SUFFIX;

sos_time_t sos_time_microseconds(const uint32_t microseconds) SOS_API_SUFFIX;

sos_time_t sos_time_nanoseconds(const uint64_t nanoseconds) SOS_API_SUFFIX;

error_t sos_get_time(sos_time_t * const time) SOS_API_SUFFIX;

error_t sos_sleep(const sos_time_t * const time) SOS_API_SUFFIX;

#endif /* SOS_API_TIME_H_ */
