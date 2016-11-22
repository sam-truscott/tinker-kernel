/*
 * mem_section.h
 *
 *  Created on: 23 Jul 2013
 *      Author: struscott
 */

#ifndef MEM_SECTION_H_
#define MEM_SECTION_H_

#include "mem_section_private.h"

mem_section_t * mem_sec_create(
		mem_pool_info_t * const pool,
		const mem_t real_addr,
		const mem_t virt_addr,
		const mem_t size,
		const mmu_memory_t mem_type,
		const mmu_privilege_t mem_priv,
		const mmu_access_t mem_access);

void mem_sec_delete(const mem_section_t * const section);
mem_t mem_sec_get_real_addr(const mem_section_t * const ms);
mem_t mem_sec_get_virt_addr(const mem_section_t * const ms);
mem_t mem_sec_get_size(const mem_section_t * const ms);
mmu_memory_t mem_sec_get_mem_type(const mem_section_t * const ms);
mmu_privilege_t mem_sec_get_priv(const mem_section_t * const ms);
mmu_access_t mem_sec_get_access(const mem_section_t * const ms);
mem_section_t * mem_sec_get_next(const mem_section_t * const ms);
void mem_sec_set_next(mem_section_t * const ms, const mem_section_t * const next);

#endif /* MEM_SECTION_H_ */
