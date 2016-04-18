/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_idle.h"
#include "tgt_types.h"
#include "tgt.h"

#pragma GCC optimize ("-Og") // easier to debug release build

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
		//tgt_wait_for_interrupt();
		tinker_wait_for_interrupt();
#if defined(IDLE_DEBUGGING)
		tinker_debug("IDLE: Awoken\n");
#endif
	}
}
