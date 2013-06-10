/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "arch/target_types.h"
#include "arch/ppc32/powerpc32_7400.h"
#include "arch/ppc32/powerpc32_mmu.h"
#include "kernel/utils/util_memcpy.h"

/**
 * Varaible used to store the stack pointer in interrupt routines
 */
EXTERN uint32_t __ivt_stack_end;
uint32_t * __ivt_stack = &__ivt_stack_end;

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
