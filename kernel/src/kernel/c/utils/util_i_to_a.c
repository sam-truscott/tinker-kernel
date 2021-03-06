/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "utils/util_i_to_a.h"

#include "utils/util_strlen.h"
#include "utils/util_reverse_string.h"
#include "utils/util_trim.h"
#include "utils/util_memset.h"

static int get_n(int * const number)
{
       const int r = (*number) % 10;
       (*number) /= 10;
       return r;
}

void util_i_to_a(const int32_t i, char buffer[], const uint32_t buffer_length)
{
	if (buffer)
	{
		if (i == 0)
		{
			/* short cut */
			buffer[0] = '0';
			buffer[1] = '\0';
		}
		else
		{
			int p = 0;

			char rev_buffer[buffer_length];
			util_memset(rev_buffer, 0, buffer_length);

			const bool_t is_neg = i < 0;
			int n = i;
			if (is_neg)
			{
			   n = i * -1;
			}
			while (n > 0)
			{
				  const int tmp = get_n(&n);
				  rev_buffer[p++] = (char)(tmp + '0');
			}
			if(is_neg)
			{
				rev_buffer[p++] = '-';
			}

			{
				const uint32_t number_length = util_strlen(rev_buffer, buffer_length);
				const uint32_t new_length = util_trim(rev_buffer, number_length);
				util_reverse_string(rev_buffer, new_length, buffer);
			}
		}
	}
}

void util_i_to_h(const uint32_t i, char buffer[], const uint32_t buffer_length)
{
	if (buffer)
	{
		if (i == 0)
		{
			buffer[0] = '0';
			buffer[1] = '\0';
		}
		else
		{
			uint32_t rem[buffer_length], length = 0;
			int32_t t = 0;
			uint32_t in = i;

			while (in>0)
			{
				rem[t]=in%16;
				in=in/16;
				t++;
				length++;
			}

			int p = 0;
			for (t=length-1 ; t>=0 ; t--)
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
