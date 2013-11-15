/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "util_i_to_a.h"

#include "util_strlen.h"
#include "util_reverse_string.h"
#include "util_trim.h"
#include "util_memset.h"

static int get_n(int * const number)
{
       const int r = (*number) % 10;
       (*number) /= 10;
       return r;
}

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
			int p = 0;

			char rev_buffer[buffer_length];
			memset(rev_buffer, 0, buffer_length);

			const unsigned char is_neg = i < 0;
			int n = i;
			if (is_neg) {
			   n = i * -1;
			   rev_buffer[p++] = '-';
			}
			while (n > 0)
			{
				  const int tmp = get_n(&n);
				  rev_buffer[p++] = (char)(tmp + '0');
			}

			{
				const uint32_t number_length = __util_strlen(rev_buffer, buffer_length);
				const uint32_t new_length = __util_trim(rev_buffer, number_length);
				__util_reverse_string(rev_buffer, new_length, buffer);
			}
		}
	}
}
#pragma weak __util_i_to_a

void __util_i_to_h(const uint32_t i, char buffer[], const uint32_t buffer_length)
{
	if ( buffer )
	{
		if ( i == 0 )
		{
			buffer[0] = '0';
			buffer[1] = '\0';
		}
		else
		{
			int p = 0;

			char rev_buffer[buffer_length];
			memset(rev_buffer, 0, buffer_length);

			int n = i;
			while (n > 0)
			{
				const int tmp = get_n(&n);
				if ( tmp <= 9 )
				{
					rev_buffer[p++] = tmp + '0';
				}
				else
				{
					rev_buffer[p++] = (tmp-10) + 'a';
				}
			}

			{
				const uint32_t number_length = __util_strlen(rev_buffer, buffer_length);
				const uint32_t new_length = __util_trim(rev_buffer, number_length);
				__util_reverse_string(rev_buffer, new_length, buffer);
			}
		}
	}
}
#pragma weak __util_i_to_h
