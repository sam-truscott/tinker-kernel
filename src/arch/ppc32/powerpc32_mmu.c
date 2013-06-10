/*************************************************************************
 *
 * SOS 3 Source Code
 * __________________
 *
 *  [2012] Samuel Steven Truscott
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

#include "powerpc32.h"
#include "powerpc32_mmu.h"

void __ppc32_add_pte(uint32_t ea, uint32_t vsid, uint32_t pte_w0, uint32_t pte_w1)
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

	if ( pPteg )
	{
		uint8_t i;
		for ( i = 0 ; i < __PPC_MAX_PTE_PER_PTEG ; i++ )
		{
			/* TODO needs to be marked as volatile in SMP system?
			 * TODO locking on the page table in (non?)SMP system? */
			__ppc32_pte_t * pte = (__ppc32_pte_t*)&pPteg[i];
			if ( pte )
			{
				if ( !(pte->w0 & 0x80000000u) )
				{
					pte->w0 = pte_w0;
					pte->w1 = pte_w1;
					break;
				}
			}
		}
	}
}
