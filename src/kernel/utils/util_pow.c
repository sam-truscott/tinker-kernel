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

#include "util_pow.h"

uint32_t __util_pow(const uint32_t a, const uint32_t b)
{
	uint32_t ret = a;

	if ( b > 1 )
	{
		for ( uint32_t it = 0 ; it < (b - 1) ; it++ )
		{
			ret *= a;
		}
	}

	return ret;
}
