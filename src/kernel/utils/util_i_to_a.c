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

#include "util_i_to_a.h"

#include "util_pow.h"
#include "util_strlen.h"
#include "util_reverse_string.h"
#include "util_trim.h"

void __util_i_to_a(const int32_t i, char buffer[], const uint32_t buffer_length)
{
	if ( buffer )
	{
		if ( i == 0 )
		{
			/* short cut */
			buffer[0] = '0';
			buffer[1] = '\0';
		}
		else
		{
			/* good old fashion way */
			char rev_buffer[buffer_length];

			for ( uint32_t t = 0 ; t < buffer_length ; t++)
			{
				rev_buffer[t] = '\0';
			}
			/*__util_memset(rev_buffer, '\0', buffer_length);*/

			uint32_t total = 0;
			for ( uint32_t power = 1 ; power < 10 ; power++ )
			{
				const uint32_t div = __util_pow(10, power);
				const uint32_t odiv = __util_pow(10, power - 1);

				uint32_t this = (uint32_t)(i % div - total);
				total += this;

				if ( power > 1 )
				{
					 this /= odiv;
				}
				rev_buffer[power-1] = (char)(this + '0');
			}

			{
				uint32_t number_length = __util_strlen(rev_buffer, buffer_length);
				__util_trim(rev_buffer, &number_length);
				__util_reverse_string(rev_buffer, number_length, buffer);
			}
		}
	}
}
