/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "utils/util_a_to_i.h"
#include "utils/util_strlen.h"

uint32_t util_a_to_i(const char buffer[], const uint32_t buffer_length)
{
	const uint32_t len = util_strlen(buffer, buffer_length);
	uint32_t value = 0;
	for (unsigned char p = 0 ; p < len ; p++)
	{
		const unsigned char pos = len - p - 1;
		const uint8_t val = buffer[pos] - '0';
		if (val <= 9)
		{
			value += (val * (p == 0 ? 1 : p * 10));
		}
	}
	return value;
}
