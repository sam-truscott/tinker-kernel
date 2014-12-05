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
#include "api/tinker_api_process.h"
#include "kernel/console/print_out.h"

//#if defined(IDLE_DEBUGGING)
#pragma GCC optimize ("-O2") // easier to debug release build
//#endif
static uint32_t kernel_idle_check(uint32_t count)
{
	if (count % 200000 == 0)
	{
#if defined (IDLE_DEBUGGING)
		tinker_debug("IDLE\n");
#endif
	}
	return count;
}

void kernel_idle(void)
{
#if defined(IDLE_DEBUGGING)
	tinker_debug("IDLE: kernel idle starting\n");
#endif
	const uint32_t testing_a = 0x55AA55AA;
	uint32_t count = 0;
	while( 1 != 2 && testing_a)
	{
		count++;
		count = kernel_idle_check(count);
	}
}
