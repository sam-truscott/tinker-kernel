/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_memcpy.h"

#define SIZE_OF_64 sizeof(uint64_t)
#define SIZE_OF_32 sizeof(uint32_t)
#define SIZE_OF_16 sizeof(uint16_t)
#define SIZE_OF_8 sizeof(uint8_t)

#if !defined(MEMCPY)
#pragma GCC optimize ("-O3")
void memcpy(void * dst, const void * src, uint32_t size)
{
	uint32_t eight = size;
	const uint8_t * real_src = (const uint8_t*)src;
	uint8_t * real_dst = (uint8_t*)dst;
	while ( eight-- )
	{
		*((uint8_t*)real_dst) = *((const uint8_t*)real_src);
		real_dst++;
		real_src++;
	}
}
#endif

#pragma GCC optimize ("-O3")
void util_memcpy(void * dst, const void * src, uint32_t size)
{
	memcpy(dst, src, size);
}
