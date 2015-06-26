/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arch/ppc32/powerpc32_7400.h"
#include "arch/ppc32/powerpc32.h"
#include "arch/ppc32/powerpc32_mmu.h"
#include "kernel/utils/util_memcpy.h"

/**
 * Varaible used to store the stack pointer in interrupt routines
 */
extern uint32_t __ivtse;
uint32_t * ivts = &__ivtse;

void tgt_initialise(void)
{
	uint32_t msr = MSR_FLAG_ME;
	/* msr |= MSR_FLAG_FP; */
	ppc_set_msr(msr);

	ppc_set_tbr(0, 0);
	for ( int i = 0 ; i < MAX_DTLB_ENTRIES ; i++ )
	{
		ppc_invalid_tlbe(i);
	}
}

void tgt_wait_for_interrupt(void)
{
	asm volatile("li %r11, 0");
	asm volatile("lis %r11, 0x20");
	asm volatile("mtspr 1008, %r11");
}
