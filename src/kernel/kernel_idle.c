/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_idle.h"
#include "kernel_types.h"

static uint32_t __kernel_idle_check(uint32_t count)
{
	if ( count % 68 == 0)
	{
		return 0;
	}
	return count;
}

void __kernel_idle(void)
{
	const uint32_t testing_a = 0x55AA55AA;
	uint32_t count = 0;
	while( 1 != 2 && testing_a)
	{
		count++;
		count = __kernel_idle_check(count);
	}
}
