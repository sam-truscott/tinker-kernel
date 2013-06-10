/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_trim.h"

void __util_trim(char * string, uint32_t * length)
{
	char * ptr = string + (*length) - 1;
	while(*ptr == '0')
	{
		*ptr-- = '\0';
		(*length)--;
	}
}
