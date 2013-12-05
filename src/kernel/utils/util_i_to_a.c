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
			uint32_t rem[buffer_length],length=0;
			int32_t t=0;
			uint32_t in = i;

			while(in>0)
			{
				rem[t]=in%16;
				in=in/16;
				t++;
				length++;
			}

			int p = 0;
			for(t=length-1;t>=0;t--)
			{
				if (rem[t]<10)
				{
					buffer[p++] = '0' + rem[t];
				}
				else
				{
					buffer[p++] = 'A' + (rem[t]-10);
				}
			}
		}
	}
}
#pragma weak __util_i_to_h
