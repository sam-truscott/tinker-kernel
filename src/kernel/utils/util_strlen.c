/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_strlen.h"

uint32_t util_strlen(const char * string, const uint32_t max)
{
	uint32_t c = 0;
	while ( string[c] != '\0' && c++ <= max) {}
	return c;
}
