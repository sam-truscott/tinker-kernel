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
	mmu_device_memory,
	mmu_random_access_memory //FIXME capitalise
} mmu_memory_t;

typedef enum
{
	mmu_no_privilege,
	mmu_user_access,
	mmu_kernel_access,
	mmu_all_access //FIXME capitalise
} mmu_privilege_t;

typedef enum
{
	mmu_no_access	= 0,
	mmu_read_only	= 1,
	mmu_read_write	= 2 //FIXME capitalise
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
