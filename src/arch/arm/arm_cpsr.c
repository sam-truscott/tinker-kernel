/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arm_cpsr.h"
#include "kernel/utils/util_memcpy.h"
#pragma GCC optimize ("-O0")
uint32_t arm_get_cpsr(void)
{
	uint32_t r;
	asm volatile("mrs %[ps], cpsr" : [ps]"=r" (r));
	return r;
}

static void arm_set_cpsr(const uint32_t r)
{
    asm volatile("msr cpsr, %[ps]" : : [ps]"r" (r));
}

void arm_enable_irq(void)
{
	arm_set_cpsr(arm_get_cpsr() & ~(1 << 7));
}

void arm_disable_irq(void)
{
	arm_set_cpsr(arm_get_cpsr() | (1 << 7));
}

void arm_disable_fiq(void)
{
	arm_set_cpsr(arm_get_cpsr() | (1 << 6));
}

psr_mode_t arm_get_psr_mode(void)
{
	return arm_get_cpsr() & 0x1F;
}

void arm_set_psr_mode(const psr_mode_t psr_mode)
{
	const uint32_t cpsr = arm_get_cpsr();
	const uint32_t masked_cpsr = cpsr & (~0x1F);
	const uint32_t new_cpsr = masked_cpsr | psr_mode;
	arm_set_cpsr(new_cpsr);
}
