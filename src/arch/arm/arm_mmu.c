/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arm_mmu.h"

#pragma GCC optimize ("-O0")

void arm_invalidate_all_tlbs(void)
{
	asm("mov r0, #0x0");
	asm("mcr p15, 0, r0, c8, c7, 0");	// invalidate all tlbs
}

void arm_disable_mmu(void)
{
	asm("mrc p15, 0, r0, c1, c0, 0");
	asm("and r0, r0, #0xFFFFEFFA");		// disable Instruction & Data cache, disable MMU
	asm("mcr p15, 0, r0, c1, c0, 0");
}

void arm_enable_mmu(void)
{
	asm("mrc p15, 0, r0, c1, c0, 0");
	asm("or r0, r0, #0x1003");			// enable Instruction & Data cache, enable MMU
	asm("mcr p15, 0, r0, c1, c0, 0");
}
