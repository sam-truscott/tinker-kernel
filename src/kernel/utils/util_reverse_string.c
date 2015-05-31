/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_reverse_string.h"

void util_reverse_string(const char * src, const uint32_t src_length, char * dst)
{
	if (src && dst)
	{
		for ( uint32_t p = 0 ; p < src_length ; p++ )
		{
			dst[p] = src[src_length-p-1];
		}
	}
}
