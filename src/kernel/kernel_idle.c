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

void kernel_idle(void)
{
#if defined(IDLE_DEBUGGING)
	tinker_debug("IDLE: kernel idle starting\n");
#endif
	while(1 != 2)
	{
#if defined(IDLE_DEBUGGING)
	tinker_debug("IDLE: Waiting for interrupt\n");
#endif
		tinker_wait_for_interrupt();
	}
}
