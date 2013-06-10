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
