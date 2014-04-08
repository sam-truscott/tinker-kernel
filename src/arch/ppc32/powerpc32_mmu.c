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

static tgt_pg_tbl_t __ppc32_get_pte(
		const tgt_pg_tbl_t * const page_tbl,
		const uint32_t ea,
		const uint32_t vsid)
{
	/* below code generates a primary PTE
	 * 06/03/2012 - checked against PPC Programming Environments Manual, page 278 */
	uint32_t hash = 0;
	uint32_t sdr1 = 0;

	uint32_t htabord;
	uint32_t htamask;
	uint32_t hash9;
	uint32_t hash10;
	tgt_pg_tbl_t pPteg;

	sdr1 = (((uint32_t)page_tbl) & 0xFFFF0000u) | 0xF;
	hash = __PPC_PRIMARY_HASH((vsid & 0xFFFFFFu), ((ea >> 12) & 0x7FFFFu));

	htabord = (sdr1 & 0xFFFF0000u);
	htamask = (sdr1 & 0x1FFu);
	htabord &= ~htamask;
	hash9 = ((hash >> 10) & htamask);
	hash10 = (hash & 0x3FFu);

	asm volatile("sync");
	pPteg = (tgt_pg_tbl_t)(htabord | (hash9 << 16) | (hash10<<6));
	return pPteg;
}

void __ppc32_add_pte(
		const tgt_pg_tbl_t * const page_tbl,
		const uint32_t ea,
		const uint32_t vsid,
		const uint32_t pte_w0,
		const uint32_t pte_w1)
{
	volatile tgt_pg_tbl_t const pPteg = __ppc32_get_pte(page_tbl, ea, vsid);

	__printp_out("__ppc32_add_pte tbl=%x, ea=%x, pteg=%x\n", page_tbl, ea, pPteg);
	if (pPteg)
	{
		uint8_t i;
		bool_t mapped = false;
		for (i = 0 ; i < __PPC_MAX_PTE_PER_PTEG ; i++)
		{
			volatile __ppc32_pte_t * const pte = &pPteg->ptes[i];
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
				else
				{
				    __printp_out("pte=%d in pbl=%x had a w0=%x, pte=%x\n", i, page_tbl, pte->w0, pte);
				}
			}
		}
		__kernel_assert("pte mapping failed", mapped);
	}
}

static void __ppc32_invalid_ea(const uint32_t ea)
{
    (void)ea;
    asm volatile("sync");
    asm volatile("tlbie %r3");
    asm volatile("eieio");
    asm volatile("tlbsync");
}

void __ppc32_remove_pte(
		const tgt_pg_tbl_t * const page_tbl,
		const uint32_t ea,
		const uint32_t vsid,
		const uint32_t pte_w0,
		const uint32_t pte_w1)
{
	volatile tgt_pg_tbl_t const pPteg = __ppc32_get_pte(page_tbl, ea, vsid);
	if (pPteg)
	{
		uint8_t i;
		bool_t unmapped = false;
		for (i = 0 ; i < __PPC_MAX_PTE_PER_PTEG ; i++)
		{
			volatile __ppc32_pte_t * const pte = &pPteg->ptes[i];
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
		__ppc32_invalid_ea(ea);
	}
}

void __ppc32_switch_page_table(
        const __process_t * const last_proc,
        const __process_t * const proc)
{
    const __mem_section_t * sec = __process_get_first_section(last_proc);
    while (sec)
    {
        __ppc32_invalid_ea(__mem_sec_get_virt_addr(sec));
        sec = __mem_sec_get_next(sec);
    }
    __ppc32_set_sdr1((((uint32_t)__process_get_page_table(proc)) & 0xFFFF0000u) | 0xF);
}
