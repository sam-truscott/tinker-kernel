/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_idle.h"
#include "arch/tgt_types.h"
#include "kernel/console/print_out.h"

#pragma GCC optimize ("-O0") // easier to debug release build
static uint32_t kernel_idle_check(uint32_t count)
{
	if ( count % 800000 == 0)
	{
		debug_print("IDLE\n");
	}
	return count;
}

void kernel_idle(void)
{
	printp_out("IDLE: kernel idle starting\n");
	const uint32_t testing_a = 0x55AA55AA;
	uint32_t count = 0;
	while( 1 != 2 && testing_a)
	{
		count++;
		count = kernel_idle_check(count);
	}
}
