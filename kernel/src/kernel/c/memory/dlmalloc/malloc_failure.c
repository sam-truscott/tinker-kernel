/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "memory/dlmalloc/malloc_failure.h"
#include "kernel_panic.h"
#include "console/print_out.h"

void malloc_failure(void)
{
	error_print("Failed to allocate memory\n");
}

void malloc_abort(void)
{
	error_print("MALLOC ABORT\n");
	kernel_panic();
}
