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

#include "kernel_assert.h"

#include "kernel_panic.h"
#include "kernel_types.h"
#include "debug/debug_print.h"

void __kernel_assert(const char * message, const bool var)
{
	__kernel_assert_cond(message, var, true);
}

void __kernel_assert_cond(const char * message, bool var, const bool cond)
{
	if ( var != cond )
	{
		__debug_print("ASSERT FAILED: %s\n", message);
		__kernel_panic();
	}
}

void __assert(const char * message, const bool var)
{
	if ( !var )
	{
		__debug_print("Assertion failed: %s\n", message);
	}
}
