/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_case.h"

void util_to_upper(char * string)
{
	char * copy = string;
	while(*copy)
	{
		if ( *copy >= 'a' && *copy <= 'z')
		{
			*copy = (char)(*copy - 32);
		}
		copy++;
	}
}

void util_to_lower(char * string)
{
	char * copy = string;
	while(*copy)
	{
		if ( *copy >= 'A' && *copy <= 'Z')
		{
			*copy = (char)(*copy + 32);
		}
		copy++;
	}
}
