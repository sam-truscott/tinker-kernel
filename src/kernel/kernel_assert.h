/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KERNEL_ASSERT_H_
#define KERNEL_ASSERT_H_

#include "kernel_types.h"

void __kernel_assert_cond(const char * message, bool var, const bool cond);

void __kernel_assert(const char * message, const bool var);

void __assert(const char * message, const bool var);

#endif /* KERNEL_ASSERT_H_ */
