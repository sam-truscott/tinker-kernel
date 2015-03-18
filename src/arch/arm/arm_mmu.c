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
#include "kernel/utils/util_memset.h"

#pragma GCC optimize ("-O0")

// http://www.embedded-bits.co.uk/2011/mmucode/
// http://www.slideshare.net/prabindh/enabling-two-level-translation-tables-in-armv7-mmu
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0198e/ch03s06s01.html

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

void arm_set_translation_table_base(const bool_t is_kernel, tgt_pg_tbl_t * const base)
{
	(void)base;
	if (is_kernel)
	{
		asm("MRC p15, 0, r1, c2, c0, 0");
	}
	else
	{
		asm("MRC p15, 0, r1, c2, c0, 1");
	}
	// write to TTBCR
	// TODO setup R0
	asm("MCR p15, 0, r0, c2, c0, 2");
	arm_invalidate_all_tlbs();
}

tgt_pg_tbl_t * tgt_initialise_page_table(mem_pool_info_t * const pool)
{
	tgt_pg_tbl_t * const table = (tgt_pg_tbl_t*)mem_alloc_aligned(pool, PAGE_TABLE_SIZE, PAGE_TABLE_ALIGNMENT);
	if (table)
	{
		util_memset(table, 0, PAGE_TABLE_SIZE);
	}
	return table;
}

error_t arm_map_memory(
		const tgt_pg_tbl_t * const table,
		const tgt_mem_t * const segment,
		const mem_section_t * const section)
{
	return NO_ERROR;
}

void arm_unmap_memory(
		const tgt_pg_tbl_t * const table,
		const tgt_mem_t * const segment,
		const mem_section_t * const section)
{

}
