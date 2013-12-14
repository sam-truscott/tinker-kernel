/*
 * mem_section.h
 *
 *  Created on: 23 Jul 2013
 *      Author: struscott
 */

#ifndef MEM_SECTION_H_
#define MEM_SECTION_H_

#include "mem_pool.h"

typedef struct __mem_section_t __mem_section_t;

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

__mem_section_t * __mem_sec_create(
		__mem_pool_info_t * const pool,
		const uint32_t real_addr,
		const uint32_t virt_addr,
		const uint32_t size,
		const mmu_memory_t mem_type,
		const mmu_privilege_t mem_priv,
		const mmu_access_t mem_access);

void __mem_sec_delete(const __mem_section_t * const section);

uint32_t __mem_sec_get_real_addr(const __mem_section_t * const ms);
uint32_t __mem_sec_get_virt_addr(const __mem_section_t * const ms);
uint32_t __mem_sec_get_size(const __mem_section_t * const ms);
mmu_memory_t __mem_sec_get_mem_type(const __mem_section_t * const ms);
mmu_privilege_t __mem_sec_get_priv(const __mem_section_t * const ms);
mmu_access_t __mem_sec_get_access(const __mem_section_t * const ms);
__mem_section_t * __mem_sec_get_next(const __mem_section_t * const ms);
void __mem_sec_set_next(__mem_section_t * const ms, const __mem_section_t * const next);

#endif /* MEM_SECTION_H_ */
