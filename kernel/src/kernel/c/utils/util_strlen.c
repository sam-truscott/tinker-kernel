/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "utils/util_strlen.h"

uint32_t util_strlen(const char * const string, const uint32_t max)
{
	uint32_t c = 0;
	if (string)
	{
		while (string[c] != '\0' && c < max)
		{
			c++;
		}
	}
	return c;
}
