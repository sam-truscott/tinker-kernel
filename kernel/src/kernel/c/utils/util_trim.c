/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "utils/util_trim.h"

uint32_t util_trim(char * const string, const uint32_t length)
{
	int new_length;
	if (string)
	{
		char * ptr = string + length - 1;
		new_length = length;
		while(*ptr == '0')
		{
			*ptr-- = '\0';
			new_length--;
		}
	}
	else
	{
		new_length = 0;
	}
	return new_length;
}
