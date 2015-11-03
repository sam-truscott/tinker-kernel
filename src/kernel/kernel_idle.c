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
#include "arch/tgt.h"

// TODO keep or remove
//#if defined(IDLE_DEBUGGING)
#pragma GCC optimize ("-Og") // easier to debug release build
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
		//tgt_wait_for_interrupt();
		tinker_wait_for_interrupt();
#if defined(IDLE_DEBUGGING)
		tinker_debug("IDLE: Awoken\n");
#endif
	}
}
