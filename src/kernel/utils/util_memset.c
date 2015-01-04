/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_memset.h"

#if !defined(MEMSET)
#pragma GCC optimize ("-O2") // this code screws up on -O3
void* memset(void * address, uint32_t value, uint32_t size)
{
	uint8_t * addr = (uint8_t*) address;
	uint32_t s = size;
	while(s--)
	{
		(*addr) = value;
		addr++;
	}
	return address;
}
#endif

#pragma GCC optimize ("-O2") // this code screws up on -O3
void* util_memset(void * address, uint32_t value, uint32_t size)
{
	return memset(address, value, size);
}
