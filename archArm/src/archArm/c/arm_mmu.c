/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arm_mmu.h"
#include "tgt_types.h"
#include "utils/util_memset.h"
#include "kernel_panic.h"
#include "console/print_out.h"

// http://www.embedded-bits.co.uk/2011/mmucode/
// http://www.slideshare.net/prabindh/enabling-two-level-translation-tables-in-armv7-mmu
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0198e/ch03s06s01.html
// http://cfile9.uf.tistory.com/image/14626F354C57C985ABA951 <-- picture

#define DEFAULT_DOMAIN 0
#define ECC_OFF 0
#define DEFAULT_TEX 1

#define ARM_MMU_SECTION_SIZE (1 * 1024 * 1024)
#define ARM_MMU_SECTION_PAGES ((ARM_MMU_SECTION_SIZE / MMU_PAGE_SIZE) - 1)


#define NUM_L2_ENTRIES 256

#pragma pack(push,1)
typedef enum arm_pg_tbl_lvl1_entry
{
	arm_pg_tbl_invalid_lvl1_entry = 0,
	arm_pg_tbl_second_level = 1,
	arm_pg_tbl_section = 2,
	arm_pg_tbl_reserved = 3
} arm_pg_tbl_lvl1_entry_t;

typedef enum arm_pg_tbl_lvl2_entry
{
	arm_pg_tbl_invalid_lvl2_entry = 0,
	arm_pg_tbl_64k_entry = 1,
	arm_pg_tbl_4k_execute_entry = 2,
	arm_pg_tbl_4k_no_execute_entry = 3
} arm_pg_tbl_lvl2_entry_t;

typedef enum arm_pg_tbl_lvl1_ng
{
	arm_pg_tbl_global = 0,
	arm_pg_tbl_process_specific = 1
} arm_pg_tbl_lvl1_ng_t;

typedef enum arm_pg_tbl_lvl1_type
{
	arm_pg_tbl_section_1mb = 0,
	arm_pg_tbl_super_section_16mb = 1
} arm_pg_tbl_lvl1_type_t ;

typedef enum arm_pg_tbl_lvl1_ns
{
	arm_pg_tbl_not_trust_zone = 0,
	arm_pg_tbl_trust_zone = 1
} arm_pg_tbl_lvl1_ns_t;

typedef enum arm_pg_tbl_lvl1_shared {
	arm_pg_tbl_not_shared = 0,
	arm_pg_tbl_shared = 1
} arm_pg_tbl_lvl1_shared_t;

typedef enum arm_pg_tbl_lvl1_apx {
	arm_pg_tbl_apx_off = 0,
	arm_pg_tbl_apx_set = 1
} arm_pg_tbl_lvl1_apx_t;

typedef enum arm_pg_tbl_lvl1_nx {
	arm_pg_tbl_execute = 0,
	arm_pg_tbl_never_execute = 1
} arm_pg_tbl_lvl1_nx_t;

typedef struct l2_tbl
{
	uint32_t l2_tbl[NUM_L2_ENTRIES];
} l2_tbl_t;
#pragma pack(pop)

#define PAGE_TABLE_ALIGNMENT 128 * 1024
#define PAGE_ENTRY_ALIGNMENT 4 * 1024

// mmu_privilege_t -> AP
static const uint8_t ap_bits[4] =
{
		0,	// MMU_NO_PRIVILEGE -> 0 (No Access)
		3,	// MMU_USER_ACCESS -> 3 (User & Kernel access)
		1, 	// MMU_KERNEL_ACCESS -> 1 (Kernel access)
		3	// MMU_ALL_ACCESS -> 3 (User & Kernel access)
};

#define ARM_GET_LVL1_SECTION_INDEX(v) \
	((v & 0xFFF00000u) >> 20)

#define ARM_GET_LVL2_PAGE_INDEX(v) \
	((v & 0x000FF000u) >> 12)

#define ARM_GET_LVL2_VIRT_ADDR(v) \
	(v & 0xFFFFF000u)

#define ARM_GET_LVL2_ENTRY(v, t) \
		&t->l2_tbl[ARM_GET_LVL2_PAGE_INDEX(v)];

static inline uint32_t arm_generate_lvl1_course(
		const l2_tbl_t * const base,
		const uint8_t p,
		const uint8_t domain,
		const arm_pg_tbl_lvl1_ns_t ns)
{
	uint32_t lvl1 = arm_pg_tbl_second_level;
	lvl1 += (ns << 3);
	lvl1 += (domain << 5);
	lvl1 += (p << 9);
	lvl1 += ((uint32_t)base & (0x3FFFFF << 10));
	return lvl1;
}

static inline uint32_t arm_generate_lvl1_section(
		const uint32_t real,
		const arm_pg_tbl_lvl1_ng_t ng,
		const arm_pg_tbl_lvl1_shared_t s,
		const arm_pg_tbl_lvl1_apx_t apx,
		const uint8_t tex,
		const uint8_t ap,
		const uint8_t p,
		const uint8_t domain,
		const arm_pg_tbl_lvl1_nx_t nx,
		const bool_t cached,
		const bool_t buffered)
{
	uint32_t lvl1 = arm_pg_tbl_section;
	lvl1 += buffered << 2;
	lvl1 += cached << 3;
	lvl1 += nx << 4;
	lvl1 += domain << 5;
	lvl1 += p << 9;
	lvl1 += ap << 10;
	lvl1 += tex << 12;
	lvl1 += apx << 15;
	lvl1 += s << 16;
	lvl1 += ng << 17;
	lvl1 += ((uint32_t)real & 0xFFF00000u);
	return lvl1;
}

static inline uint32_t arm_generate_lvl2(
		const uint32_t real,
		const arm_pg_tbl_lvl1_ng_t ng,
		const arm_pg_tbl_lvl1_shared_t s,
		const arm_pg_tbl_lvl1_apx_t apx,
		const uint8_t tex,
		const uint8_t ap,
		const bool_t cached,
		const bool_t buffered)
{
	uint32_t lvl2 = 0;
	lvl2 += arm_pg_tbl_4k_execute_entry;
	lvl2 += (buffered << 2);
	lvl2 += (cached << 3);
	lvl2 += (ap << 4);
	lvl2 += (tex << 6);
	lvl2 += (apx << 9);
	lvl2 += (s << 10);
	lvl2 += (ng << 11);
	lvl2 += ARM_GET_LVL2_VIRT_ADDR(real);
	return lvl2;
}

static l2_tbl_t * arm_get_lvl2_table(
		const uint32_t virtual,
		const bool_t create_if_missing,
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table)
{
	l2_tbl_t * entry = NULL;
	if (table)
	{
		const uint16_t virt_section = ARM_GET_LVL1_SECTION_INDEX(virtual);
		if (table->lvl1_entry[virt_section] == 0 && create_if_missing)
		{
			entry = mem_alloc_aligned(pool, sizeof(l2_tbl_t), PAGE_ENTRY_ALIGNMENT);
			if (entry)
			{
				util_memset(entry, 0, sizeof(l2_tbl_t));
				table->lvl1_entry[virt_section] = arm_generate_lvl1_course(
						entry,
						ECC_OFF,
						DEFAULT_DOMAIN,
						arm_pg_tbl_not_trust_zone);
			}
			else
			{
				kernel_panic();
			}
		}
		entry = (l2_tbl_t*)(ARM_GET_LVL2_VIRT_ADDR(table->lvl1_entry[virt_section]));
		// if it's a section entry rather than a course entry return null
		if (((uint32_t)entry & arm_pg_tbl_section) == arm_pg_tbl_section)
		{
			entry = NULL;
		}
	}
	return entry;
}

static bool_t arm_is_1mb_section(
		const uint32_t start,
		const uint32_t end)
{
	// do we start on a 1mb boundary
	if ((start % ARM_MMU_SECTION_SIZE) != 0)
	{
		return false;
	}
	// is the end more than 1Mb away from the start?
	if ((end < (start + ARM_MMU_SECTION_SIZE)))
	{
		return false;
	}
	// size is at least 1mb
	return true;
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

static void arm_map_section(
		const mem_section_t * const section,
		const uint32_t virt,
		const uint32_t real,
		tgt_pg_tbl_t* const table)
{
	const mmu_memory_t mem_type = mem_sec_get_mem_type(section);
	const mmu_privilege_t priv = mem_sec_get_priv(section);
	const mmu_access_t acc = mem_sec_get_access(section);
	table->lvl1_entry[ARM_GET_LVL1_SECTION_INDEX(virt)] = arm_generate_lvl1_section(
			real,
			arm_pg_tbl_process_specific,
			arm_pg_tbl_not_shared,
			(acc == MMU_READ_ONLY),
			DEFAULT_TEX,
			ap_bits[priv],
			ECC_OFF,
			DEFAULT_DOMAIN,
			arm_pg_tbl_execute, // TODO default to execute - may wish to change in future
			(mem_type == MMU_RANDOM_ACCESS_MEMORY),
			(mem_type == MMU_RANDOM_ACCESS_MEMORY));
}

static return_t arm_map_page(
		const mem_section_t * const section,
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const uint32_t virt,
		const uint32_t real)
{
	l2_tbl_t * const lvl2_tbl = arm_get_lvl2_table(virt, true, pool, table);
	if (lvl2_tbl)
	{
		uint32_t * const lvl2_entry = ARM_GET_LVL2_ENTRY(virt, lvl2_tbl);
		if (lvl2_entry)
		{
			const mmu_privilege_t priv = mem_sec_get_priv(section);
			const mmu_access_t acc = mem_sec_get_access(section);
			const mmu_memory_t mem_type = mem_sec_get_mem_type(section);
			*lvl2_entry = arm_generate_lvl2(
					real,
					arm_pg_tbl_process_specific,
					arm_pg_tbl_not_shared,
					(acc == MMU_READ_ONLY),
					DEFAULT_TEX,
					ap_bits[priv],
					(mem_type == MMU_RANDOM_ACCESS_MEMORY),
					(mem_type == MMU_RANDOM_ACCESS_MEMORY));
			return NO_ERROR;
		}
		else
		{
			return OUT_OF_MEMORY;
		}
	}
	else
	{
		return OUT_OF_MEMORY;
	}
}

return_t arm_map_memory(
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section)
{
	if (table && section)
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
			const uint32_t end = virt + (pages * MMU_PAGE_SIZE);
			const uint32_t real = mem_sec_get_real_addr(section) + (page * MMU_PAGE_SIZE);
			if (arm_is_1mb_section(virt, end))
			{
				arm_map_section(
						section,
						virt,
						real,
						table);
				page += ARM_MMU_SECTION_PAGES;
			}
			else
			{
				const return_t ret = arm_map_page(
						section,
						pool,
						table,
						virt,
						real);
				if (ret != NO_ERROR)
				{
					return ret;
				}
			}
		}
	}
	else
	{
		return PARAMETERS_NULL;
	}
	return NO_ERROR;
}

void arm_unmap_memory(
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section)
{
	if (table && section)
	{
		const uint32_t virt = mem_sec_get_virt_addr(section);
		l2_tbl_t * const lvl2_tbl = arm_get_lvl2_table(virt, true, pool, table);
		if (lvl2_tbl)
		{
			uint32_t * const lvl2_entry = ARM_GET_LVL2_ENTRY(virt, lvl2_tbl);
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

void arm_print_page_table(tgt_pg_tbl_t * const table)
{
	debug_prints(TARGET, "Print page table\n");
	for (uint16_t section = 0 ; section < NUM_L1_ENTRIES ; section++)
	{
		if ((table->lvl1_entry[section] & 1) == 1)
		{
			debug_print(TARGET, "%d: Course table\n", section);
			l2_tbl_t * const lvl2 = arm_get_lvl2_table(
					section * ARM_MMU_SECTION_SIZE,
					false,
					NULL,
					table);
			for (uint16_t page = 0 ; page < NUM_L2_ENTRIES ; page++)
			{
				if ((lvl2->l2_tbl[page] & 1) == 1)
				{
					debug_prints(TARGET, "64K entry\n");
				}
				else if ((lvl2->l2_tbl[page] & 2) == 2)
				{
					debug_print(TARGET, "%x -> %x (c=%d, b=%d, ap=%d, tex=%d)\n",
							(section * ARM_MMU_SECTION_SIZE) + (page * MMU_PAGE_SIZE),
							lvl2->l2_tbl[page] & 0xFFFFF000,
							(lvl2->l2_tbl[page] & 0x4) >> 2,
							(lvl2->l2_tbl[page] & 0x8) >> 3,
							(lvl2->l2_tbl[page] & 0x30) >> 4,
							(lvl2->l2_tbl[page] >> 6) & 0x7);
				}
			}
		}
		else if ((table->lvl1_entry[section] & 2) == 2)
		{
			debug_print(TARGET, "%d: Section entry\n", section);
		}
	}
}

#pragma GCC optimize ("-O0")

void arm_invalidate_all_tlbs(void)
{
	asm volatile("mov r0, #0x0");
	asm volatile("mcr p15, 0, r0, c8, c7, 0");	// invalidate all tlbs
}

void arm_disable_mmu(void)
{
	asm volatile("mrc p15, 0, r0, c1, c0, 0");
	asm volatile("ldr r1, =0x1005");
	asm volatile("bic r0, r0, r1");		// disable Instruction & Data cache, disable MMU
	asm volatile("mcr p15, 0, r0, c1, c0, 0");
}

void arm_enable_mmu(const bool_t cache)
{
	// domain
	asm volatile("mov r0, #0x3");
	asm volatile("mcr p15, 0, r0, c3, c0, 0");
	asm volatile("mrc p15, 0, r0, c1, c0, 0");
	if (cache)
	{
		asm volatile("ldr r1, =0x1005");
	}
	else
	{
		asm volatile("ldr r1, =0x0001");
	}
	asm volatile("orr r0, r0, r1");			// enable Instruction & Data cache, enable MMU
	asm volatile("mcr p15, 0, r0, c1, c0, 0");
}

static inline void arm_set_translation_control(const uint32_t ctl)
{
	(void)ctl;
	asm volatile("mcr p15, 0, r0, c2, c0, 2"); 	// TTBCR, r0 -> ctl
}

void arm_set_domain_access_register(const uint32_t dar)
{
	(void)dar;
	asm volatile("mcr p15, 0, r0, c3, c0, 0"); 	// TTBCR, r0 -> dar
}

void arm_set_translation_table_base(tgt_pg_tbl_t * const base)
{
	(void)base;
	asm volatile("" ::: "memory");
	asm volatile("mcr p15, 0, r0, c2, c0, 0");	// TTBR0, r0 -> base
	arm_set_translation_control(0);
}

uint32_t arm_get_cp15_c1(void)
{
	uint32_t r;
	asm volatile("mrc p15, 0, %[ps], c1, c0, 0" : [ps]"=r" (r));
	return r;
}
