/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_trim.h"

uint32_t __util_trim(char * const string, const uint32_t length)
{
	char * ptr = string + length - 1;
	int new_length = length;
	while(*ptr == '0')
	{
		*ptr-- = '\0';
		new_length--;
	}
	return new_length;
}
#pragma weak __util_trim
