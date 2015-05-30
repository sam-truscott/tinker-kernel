/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_case.h"

void util_to_upper(char * const string)
{
	if (string)
	{
		char * copy = string;
		char v = *copy;
		while(v)
		{
			if ( v >= 'a' && v <= 'z')
			{
				*copy = (char)(v - 32);
			}
			v = *(++copy);
		}
	}
}

void util_to_lower(char * const string)
{
	if (string)
	{
		char * copy = string;
		char v = *copy;
		while(v)
		{
			if ( v >= 'A' && v <= 'Z')
			{
				*copy = (char)(v + 32);
			}
			v = *(++copy);
		}
	}
}
