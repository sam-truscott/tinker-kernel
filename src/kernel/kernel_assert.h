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

#ifndef KERNEL_ASSERT_H_
#define KERNEL_ASSERT_H_

#include "kernel_types.h"

void __kernel_assert_cond(const char * message, bool var, const bool cond);

void __kernel_assert(const char * message, const bool var);

void __assert(const char * message, const bool var);

#endif /* KERNEL_ASSERT_H_ */
