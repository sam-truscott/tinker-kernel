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

#include "basic_hashes.h"

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
	int8_t * val = (int8_t*)ptr;

	for (uint32_t i = 0; i < size; i++) {
		h = 31*h + val[off++];
	}
	return h;
}
