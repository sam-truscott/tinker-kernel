/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arm_cpsr.h"
#include "utils/util_memcpy.h"
#pragma GCC optimize ("-O0")

void arm_enable_irq(void)
{
	__asm__ volatile("msr daifclr, #2" ::: "memory");
	// DAIF bits: [3: Debug, 2: SError, 1: IRQ, 0: FIQ]
	// #2 clears IRQ mask
}

void arm_disable_irq(void)
{
	__asm__ volatile("msr daifset, #2" ::: "memory");
	// sets IRQ mask
}

void arm_disable_fiq(void)
{
	__asm__ volatile("msr daifset, #1" ::: "memory");
	// sets FIQ mask
}
