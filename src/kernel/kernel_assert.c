/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_assert.h"

#include "kernel_panic.h"
#include "console/print_out.h"

void kernel_assert(const char * message, const bool_t var)
{
	kernel_assert_cond(message, var, true);
}

void kernel_assert_cond(const char * message, const bool_t var, const bool_t cond)
{
	if (var != cond)
	{
		error_print("ASSERT FAILED: %s\n", message);
		kernel_panic();
	}
}

