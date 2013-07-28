/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_assert.h"

#include "kernel_panic.h"
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
