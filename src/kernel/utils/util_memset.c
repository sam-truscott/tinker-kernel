/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_memset.h"

#pragma GCC optimize ("-O2") // this code screws up on -O3
void* memset(void * address, const uint32_t value, const uint32_t size)
{
	return __builtin_memset(address, value, size);
}
