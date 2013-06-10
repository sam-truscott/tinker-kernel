/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_memset.h"

void __util_memset(void * address, uint8_t value, uint32_t size)
{
	uint8_t * addr = (uint8_t*) address;
	while(size--)
	{
		(*addr) = value;
		addr++;
	}
}
