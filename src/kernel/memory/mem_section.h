/*
 * mem_section.h
 *
 *  Created on: 23 Jul 2013
 *      Author: struscott
 */

#ifndef MEM_SECTION_H_
#define MEM_SECTION_H_

#include "mem_pool.h"

typedef struct mem_section_t mem_section_t;

typedef enum
{
	MMU_DEVICE_MEMORY,
	MMU_RANDOM_ACCESS_MEMORY
} mmu_memory_t;

typedef enum
{
	MMU_NO_PRIVILEGE,
	MMU_USER_ACCESS,
	MMU_KERNEL_ACCESS,
	MMU_ALL_ACCESS
} mmu_privilege_t;

typedef enum
{
	MMU_NO_ACCESS	= 0,
	MMU_READ_ONLY	= 1,
	MMU_READ_WRITE	= 2
} mmu_access_t;

mem_section_t * mem_sec_create(
		mem_pool_info_t * const pool,
		const uint32_t real_addr,
		const uint32_t virt_addr,
		const uint32_t size,
		const mmu_memory_t mem_type,
		const mmu_privilege_t mem_priv,
		const mmu_access_t mem_access);

void mem_sec_delete(const mem_section_t * const section);

uint32_t mem_sec_get_real_addr(const mem_section_t * const ms);
uint32_t mem_sec_get_virt_addr(const mem_section_t * const ms);
uint32_t mem_sec_get_size(const mem_section_t * const ms);
mmu_memory_t mem_sec_get_mem_type(const mem_section_t * const ms);
mmu_privilege_t mem_sec_get_priv(const mem_section_t * const ms);
mmu_access_t mem_sec_get_access(const mem_section_t * const ms);
mem_section_t * mem_sec_get_next(const mem_section_t * const ms);
void mem_sec_set_next(mem_section_t * const ms, const mem_section_t * const next);

#endif /* MEM_SECTION_H_ */
