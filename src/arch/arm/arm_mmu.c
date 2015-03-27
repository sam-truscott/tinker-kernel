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
#include "kernel/kernel_panic.h"

#pragma GCC optimize ("-O0")

// http://www.embedded-bits.co.uk/2011/mmucode/
// http://www.slideshare.net/prabindh/enabling-two-level-translation-tables-in-armv7-mmu
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0198e/ch03s06s01.html
// http://cfile9.uf.tistory.com/image/14626F354C57C985ABA951 <-- picture

#define DEFAULT_DOMAIN 0
#define ECC_OFF 0
#define DEFAULT_TEX 0

// mmu_privilege_t -> AP
static const uint8_t ap_bits[4] =
{
		0,	// MMU_NO_PRIVILEGE -> 0 (No Access)
		3,	// MMU_USER_ACCESS -> 3 (User & Kernel access)
		1, 	// MMU_KERNEL_ACCESS -> 1 (Kernel access)
		3	// MMU_ALL_ACCESS -> 3 (User & Kernel access)
};

void arm_invalidate_all_tlbs(void)
{
	asm("mov r0, #0x0");
	asm("mcr p15, 0, r0, c8, c7, 0");	// invalidate all tlbs
}

void arm_disable_mmu(void)
{
	asm("mrc p15, 0, r0, c1, c0, 0");
	asm("ldr r1, =0x1005");
	asm("bic r0, r0, r1");		// disable Instruction & Data cache, disable MMU
	asm("mcr p15, 0, r0, c1, c0, 0");
}

void arm_enable_mmu(void)
{
	asm("mrc p15, 0, r0, c1, c0, 0");
	asm("ldr r1, =0x1003");
	asm("orr r0, r0, r1");			// enable Instruction & Data cache, enable MMU
	asm("mcr p15, 0, r0, c1, c0, 0");

	// check
	asm("mrc p15, 0, r0, c1, c0, 0");

	// domain
	asm("mov r0, #0x3");
	asm("mcr p15, 0, r0, c3, c0, 0");
}

static inline void arm_set_translation_control(const uint32_t ctl)
{
	(void)ctl;
	asm("MCR p15, 0, r0, c2, c0, 2"); // TTBCR
}

void arm_set_translation_table_base(const bool_t is_kernel, tgt_pg_tbl_t * const base)
{
	(void)base;
	if (is_kernel)
	{
		asm("MRC p15, 0, r1, c2, c0, 1"); 	// TTBR1 - Kernel
		arm_set_translation_control(2);		// TODO confirm this is correct for kernel
	}
	else
	{
		asm("MRC p15, 0, r1, c2, c0, 0"); 	// TTBR0 - User
		arm_set_translation_control(0);		// default to TTBR0 always
	}
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

static inline uint32_t arm_generate_lvl1(
		const l2_tbl_t * const base,
		const uint8_t p,
		const uint8_t domain,
		const arm_pg_tbl_lvl1_nx_t nx,
		const bool_t cached,
		const bool_t buffered,
		const arm_pg_tbl_lvl1_entry_t lvl1_type)
{
	uint32_t lvl1 = 0;
	lvl1 += lvl1_type;
	lvl1 += (buffered << 2);
	lvl1 += (cached << 3);
	lvl1 += (nx << 4);
	lvl1 += (domain << 5);
	lvl1 += (p << 9);
	lvl1 += ((uint32_t)base & (0x3FFFFF << 10));
	return lvl1;
}

#define ARM_GET_LVL1_SECTION_INDEX(v) \
	((v & 0xFFF00000u) >> 20)

#define ARM_GET_LVL2_PAGE_INDEX(v) \
	((v & 0x000FF000u) >> 12)

#define ARM_GET_LVL2_VIRT_ADDR(v) \
	(v & 0xFFFFF000u)

static inline uint32_t arm_generate_lvl2(
		const uint32_t virt,
		const arm_pg_tbl_lvl1_ng_t ng,
		const arm_pg_tbl_lvl1_shared_t s,
		const arm_pg_tbl_lvl1_apx_t apx,
		const uint8_t tex,
		const uint8_t ap,
		const bool_t cached,
		const bool_t buffered)
{
	uint32_t lvl2 = 0;
	lvl2 += (buffered << 2);
	lvl2 += (cached << 3);
	lvl2 += (ap << 4);
	lvl2 += (tex << 6);
	lvl2 += (apx << 9);
	lvl2 += (s << 10);
	lvl2 += (ng << 11);
	lvl2 += ARM_GET_LVL2_VIRT_ADDR(virt);
	lvl2 += arm_pg_tbl_4k_execute_entry;
	return lvl2;
}

static l2_tbl_t * arm_get_lvl2_table(
		const uint32_t virtual,
		const bool_t create_if_missing,
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section)
{
	l2_tbl_t * entry = NULL;
	if (table)
	{
		const uint32_t virt_section = ARM_GET_LVL1_SECTION_INDEX(virtual);
		if (table->lvl1_entry[virt_section] == 0 && create_if_missing)
		{
			entry = mem_alloc_aligned(pool, sizeof(l2_tbl_t), PAGE_TABLE_ALIGNMENT);
			if (entry)
			{
				util_memset(entry, 0, sizeof(l2_tbl_t));
				const mmu_memory_t mem_type = mem_sec_get_mem_type(section);
				table->lvl1_entry[virt_section] = arm_generate_lvl1(
						entry,
						ECC_OFF,
						DEFAULT_DOMAIN,
						arm_pg_tbl_execute, // TODO default to execute - may wish to change in future
						(mem_type == MMU_RANDOM_ACCESS_MEMORY),
						(mem_type == MMU_RANDOM_ACCESS_MEMORY),
						arm_pg_tbl_second_level);
			}
			else
			{
				kernel_panic();
			}
		}
		entry = (l2_tbl_t*)(ARM_GET_LVL2_VIRT_ADDR(table->lvl1_entry[virt_section]));
	}
	return entry;
}

static inline uint32_t * arm_get_lvl2_entry(
		const uint32_t virtual,
		l2_tbl_t * const table)
{
	const uint32_t virt_page = ARM_GET_LVL2_PAGE_INDEX(virtual);
	return &table->l2_tbl[virt_page];
}

error_t arm_map_memory(
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section)
{
	if (table)
	{
		const uint32_t size = mem_sec_get_size(section);
		uint32_t pages = size / MMU_PAGE_SIZE;
		if ((size % MMU_PAGE_SIZE) !=0)
		{
			pages++;
		}
		for (uint32_t page = 0 ; page < pages ; page++)
		{
			const uint32_t virt = mem_sec_get_virt_addr(section) + (page * MMU_PAGE_SIZE);
			l2_tbl_t * const lvl2_tbl = arm_get_lvl2_table(virt, true, pool, table, section);
			if (lvl2_tbl)
			{
				uint32_t * const lvl2_entry = arm_get_lvl2_entry(virt, lvl2_tbl);
				if (lvl2_entry)
				{
					const mmu_privilege_t priv = mem_sec_get_priv(section);
					const mmu_access_t acc = mem_sec_get_access(section);
					*lvl2_entry = arm_generate_lvl2(
							virt,
							arm_pg_tbl_not_trust_zone,
							arm_pg_tbl_section_1mb,
							arm_pg_tbl_process_specific,
							arm_pg_tbl_not_shared,
							(acc == MMU_READ_ONLY),
							DEFAULT_TEX,
							ap_bits[priv]);
				}
				else
				{
					// TODO error
					kernel_panic();
				}
			}
			else
			{
				// TODO error
				kernel_panic();
			}
		}
	}
	else
	{
		// TODO error
		kernel_panic();
	}
	return NO_ERROR;
}

void arm_unmap_memory(
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section)
{
	if (table)
	{
		const uint32_t virt = mem_sec_get_virt_addr(section);
		l2_tbl_t * const lvl2_tbl = arm_get_lvl2_table(virt, true, pool, table, section);
		if (lvl2_tbl)
		{
			uint32_t * const lvl2_entry = arm_get_lvl2_entry(virt, lvl2_tbl);
			if (lvl2_entry)
			{
				*lvl2_entry = 0;
			}
			else
			{
				// TODO error
			}
		}
		else
		{
			// TODO error
		}
	}
	else
	{
		// TODO error
	}
}
