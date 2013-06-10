/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "util_case.h"

void __util_to_upper(char * string)
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

void __util_to_lower(char * string)
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
