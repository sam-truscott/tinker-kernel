/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "util_strcpy.h"

#include "util_strlen.h"
#include "util_memcpy.h"

char * util_strcpy (char * destination, const char * source, const uint32_t max)
{
	util_memcpy(destination, source, util_strlen(source, max));
	return destination;
}
