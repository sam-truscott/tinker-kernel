/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "util_memcpy.h"

#define SIZE_OF_64 sizeof(uint64_t)
#define SIZE_OF_32 sizeof(uint32_t)
#define SIZE_OF_16 sizeof(uint16_t)
#define SIZE_OF_8 sizeof(uint8_t)

extern inline void * memcpy(void * dst, const void * src, const uint32_t size) {
	__util_memcpy(dst, src, size);
	return dst;
}

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

	uint8_t * real_src = (uint8_t*)src;
	uint8_t * real_dst = (uint8_t*)dst;

	while ( sixty-- )
	{
		*((uint64_t*)real_dst) = *((uint64_t*)real_src);
		real_dst+=SIZE_OF_64;
		real_src+=SIZE_OF_64;
	}

	while ( thirty-- )
	{
		*((uint32_t*)real_dst) = *((uint32_t*)real_src);
		real_dst+=SIZE_OF_32;
		real_src+=SIZE_OF_32;
	}

	while ( sixteen-- )
	{
		*((uint16_t*)real_dst) = *((uint16_t*)real_src);
		real_dst+=SIZE_OF_16;
		real_src+=SIZE_OF_16;
	}

	while ( eight-- )
	{
		*((uint8_t*)real_dst) = *((uint8_t*)real_src);
		real_dst++;
		real_src++;
	}
}
