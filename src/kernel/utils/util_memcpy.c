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
void util_memcpy(void * dst, const void * src, const uint32_t size)
{
	__builtin_memcpy(dst, src, size);
}

void memcpy(void * dst, const void * src, const uint32_t size)
{
	util_memcpy(dst, src, size);
}
