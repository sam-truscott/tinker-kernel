/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arm_mmu.h"
#include "arch/tgt_types.h"

#pragma GCC optimize ("-O0")

static const uint32_t ENABLE_CACHE_MMU = 0x1003;
static const uint32_t DISABLE_CACHE_MMU = 0xFFFFEFFA;

void arm_invalidate_all_tlbs(void)
{
	asm("mov r0, #0x0");
	asm("mcr p15, 0, r0, c8, c7, 0");	// invalidate all tlbs
}

void arm_disable_mmu(void)
{
	asm("mrc p15, 0, r0, c1, c0, 0");
	asm("ldr r1, =DISABLE_CACHE_MMU");
	asm("and r0, r0, r1");		// disable Instruction & Data cache, disable MMU
	asm("mcr p15, 0, r0, c1, c0, 0");
}

void arm_enable_mmu(void)
{
	asm("mrc p15, 0, r0, c1, c0, 0");
	asm("ldr r1, =ENABLE_CACHE_MMU");
	asm("orr r0, r0, r1");			// enable Instruction & Data cache, enable MMU
	asm("mcr p15, 0, r0, c1, c0, 0");
}
