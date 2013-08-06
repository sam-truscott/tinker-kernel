/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_pow.h"

uint32_t __util_pow(const uint32_t a, const uint32_t b)
{
	uint32_t ret = a;

	if ( b > 1 )
	{
		for ( uint32_t it = 0 ; it < (b - 1) ; it++ )
		{
			ret *= a;
		}
	}

	return ret;
}
#pragma weak __util_pow
