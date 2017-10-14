/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "utils/util_memset.h"

#if defined(MEMSET)
#include <string.h>
#else
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

void* util_memset(void * address, uint32_t value, uint32_t size)
{
	return memset(address, value, size);
}
