/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_memset.h"

#pragma GCC optimize ("-O2") // this code screws up on -O3
void* memset(void * address, uint32_t value, uint32_t size)
{
	uint8_t * addr = (uint8_t*) address;
	while(size--)
	{
		(*addr) = value;
		addr++;
	}
	return address;
}
#pragma weak memset
