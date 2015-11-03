/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "util_strcat.h"

#include "util_strlen.h"
#include "util_memcpy.h"
#include "tinker_api.h"

char * util_strcat (char * destination, const char * source, const uint32_t max)
{
	const uint32_t existing = util_strlen(destination, max);
	char * const buff = destination + existing;
	util_memcpy(buff, source, util_strlen(source, max - existing));
	return destination;
}
