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
