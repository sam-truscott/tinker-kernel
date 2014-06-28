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

#pragma GCC optimize ("-O3")
void __util_memcpy(void * dst, const void * src, const uint32_t size)
{
	uint32_t total = size;
	uint32_t sixty = total / SIZE_OF_64;
	total -= sixty * SIZE_OF_64;
	uint32_t thirty = total / SIZE_OF_32;
	total -= thirty * SIZE_OF_32;
	uint32_t sixteen = total / SIZE_OF_16;
	total -= sixteen * 2;
	uint32_t eight = total;

	/*
	 * TODO this needs to take into account that the first
	 * start of the copy might not be word aligned therefore
	 * all copies would be mis-aligned creating a massive
	 * performance hit
	 */

	const uint8_t * real_src = (const uint8_t*)src;
	uint8_t * real_dst = (uint8_t*)dst;

	while ( sixty-- )
	{
		*((uint64_t*)real_dst) = *((const uint64_t*)real_src);
		real_dst+=SIZE_OF_64;
		real_src+=SIZE_OF_64;
	}

	while ( thirty-- )
	{
		*((uint32_t*)real_dst) = *((const uint32_t*)real_src);
		real_dst+=SIZE_OF_32;
		real_src+=SIZE_OF_32;
	}

	while ( sixteen-- )
	{
		*((uint16_t*)real_dst) = *((const uint16_t*)real_src);
		real_dst+=SIZE_OF_16;
		real_src+=SIZE_OF_16;
	}

	while ( eight-- )
	{
		*((uint8_t*)real_dst) = *((const uint8_t*)real_src);
		real_dst++;
		real_src++;
	}
}
#pragma weak __util_memcpy

extern void memcpy(void * dst, const void * src, const uint32_t size)
{
	__util_memcpy(dst, src, size);
}
