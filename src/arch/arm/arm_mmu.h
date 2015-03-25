/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef ARCH_ARM_ARM_MMU_H_
#define ARCH_ARM_ARM_MMU_H_

#include "tgt_mem.h"
#include "kernel/memory/mem_section.h"
#include "tinker_api_errors.h"

void arm_invalidate_all_tlbs(void);

void arm_disable_mmu(void);

void arm_enable_mmu(void);

void arm_set_translation_table_base(const bool_t is_kernel, tgt_pg_tbl_t * const base);

error_t arm_map_memory(
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section);

void arm_unmap_memory(
		mem_pool_info_t * const pool,
		tgt_pg_tbl_t * const table,
		const mem_section_t * const section);

#endif /* ARCH_ARM_ARM_MMU_H_ */
