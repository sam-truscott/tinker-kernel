/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "basic_hashes.h"
#include "kernel/utils/util_streq.h"

#define MAX_HASH_KEY_LEN 255

int32_t __hash_basic_integer(const void * ptr, const uint32_t size)
{
	int32_t hash;
	switch(size)
	{
		case sizeof(int32_t):
			hash = *((int32_t*)ptr);
			break;
		case sizeof(int16_t):
			hash = *((int16_t*)ptr);
			break;
		case sizeof(int8_t):
			hash = *((int8_t*)ptr);
			break;
		default:
			hash = 0;
			break;
	}
	return hash;
}

int32_t __hash_basic_string(const void * ptr, const uint32_t size)
{
	int32_t h = 0;
	int32_t off = 0;
	char * val = (char*)ptr;

	for (uint32_t i = 0; i < size; i++)
	{
		h = 31*h + val[off++];
	}
	return h;
}

bool_t __hash_equal_integer(const uint32_t l, const uint32_t r)
{
	return (l == r);
}

bool_t __hash_equal_string(const char * const l, const char * const r)
{
	return __util_streq(l, r, MAX_HASH_KEY_LEN);
}
