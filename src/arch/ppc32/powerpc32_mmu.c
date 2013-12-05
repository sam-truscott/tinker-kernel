/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "powerpc32.h"
#include "powerpc32_mmu.h"
#include "kernel/kernel_assert.h"

static __ppc32_pteg_t * __ppc32_get_pte(uint32_t ea, uint32_t vsid)
{
	/* below code generates a primary PTE
	 * 06/03/2012 - checked against PPC Programming Environments Manual, page 278 */
	uint32_t hash = 0;
	uint32_t sdr1 = 0;

	uint32_t htabord;
	uint32_t htamask;
	uint32_t hash9;
	uint32_t hash10;
	__ppc32_pteg_t * pPteg = 0;

	sdr1 = __ppc32_get_sdr1();
	/* FIXME the macro can do the below masking/shifting */
	hash = __PPC_PRIMARY_HASH((vsid & 0xFFFFFFu), ((ea >> 12) & 0x7FFFFu));

	htabord = (sdr1 & 0xFFFF0000u);
	htamask = (sdr1 & 0x1FFu);
	htabord &= ~htamask;
	hash9 = ((hash >> 10) & htamask);
	hash10 = (hash & 0x3FFu);

	pPteg = (__ppc32_pteg_t*)(htabord | (hash9 << 16) | (hash10<<6));
	return pPteg;
}

void __ppc32_add_pte(uint32_t ea, uint32_t vsid, uint32_t pte_w0, uint32_t pte_w1)
{
	volatile __ppc32_pteg_t * const pPteg = __ppc32_get_pte(ea, vsid);

	if ( pPteg )
	{
		uint8_t i;
		bool_t mapped = false;
		for ( i = 0 ; i < __PPC_MAX_PTE_PER_PTEG ; i++ )
		{
			/* TODO needs to be marked as volatile in SMP system?
			 * TODO locking on the page table in (non?)SMP system? */
			volatile __ppc32_pte_t * pte = (__ppc32_pte_t*)&pPteg[i];
			if (pte)
			{
				if (!(pte->w0 & 0x80000000u))
				{
					pte->w0 = pte_w0;
					pte->w1 = pte_w1;
					asm volatile("sync");
					mapped = true;
					break;
				}
			}
		}
		__kernel_assert("pte mapping failed", mapped);
	}
}

void __ppc32_remove_pte(uint32_t ea, uint32_t vsid, uint32_t pte_w0, uint32_t pte_w1)
{
	asm volatile("sync");
	asm volatile("tlbie %r3");
	asm volatile("eieio");
	asm volatile("tlbsync");
	volatile __ppc32_pteg_t * const pPteg = __ppc32_get_pte(ea, vsid);
	if ( pPteg )
	{
		uint8_t i;
		bool_t unmapped = false;
		for ( i = 0 ; i < __PPC_MAX_PTE_PER_PTEG ; i++ )
		{
			volatile __ppc32_pte_t * pte = (__ppc32_pte_t*)&pPteg[i];
			if ( pte )
			{
				if (pte->w0 & 0x80000000u)
				{
					uint32_t w1 = pte->w1;
					w1 &= ~(1 << 7); //C
					if ((pte->w0 == pte_w0) && (w1 == pte_w1))
					{
						unmapped = true;
						pte->w1 &= ~3; 			 // clear access
						pte->w0 &= ~0x80000000u; // disable
						asm volatile("sync");
						break;
					}
				}
			}
		}
		__kernel_assert("pte unmapping failed", unmapped);
	}
}
