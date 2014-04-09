/*
 *
 * TINKER Source Code
 * __________________
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
uint32_t * __ivts = &__ivtse;

void __tgt_initialise(void)
{
	uint32_t msr = MSR_FLAG_ME;
	/* msr |= MSR_FLAG_FP; */
	__ppc_set_msr(msr);

	__ppc_set_tbr(0, 0);
	for ( int i = 0 ; i < MAX_DTLB_ENTRIES ; i++ )
	{
		__ppc_invalid_tlbe(i);
	}
}
