/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "mem_section.h"

#include "kernel/kernel_assert.h"

typedef struct mem_section_t
{
	mem_section_t *next;
	uint32_t 			real_address;
	uint32_t			virt_address;
	uint32_t 			size;
	mmu_memory_t		memory_type;
	mmu_privilege_t		privilege;
	mmu_access_t		access_rights;
	__mem_pool_info_t * pool;
} mem_section_internal_t;

const mem_section_t * __mem_sec_create(
		__mem_pool_info_t * const pool,
		const uint32_t real_addr,
		const uint32_t virt_addr,
		const uint32_t size,
		const mmu_memory_t mem_type,
		const mmu_privilege_t mem_priv,
		const mmu_access_t mem_access)
{
	mem_section_t * ms = __mem_alloc(pool, sizeof(mem_section_t));
	if (ms)
	{
		ms->real_address = real_addr;
		ms->virt_address = virt_addr;
		ms->size = size;
		ms->memory_type = mem_type;
		ms->privilege = mem_priv;
		ms->access_rights = mem_access;
		ms->pool = pool;
	}
	return ms;
}

void __mem_sec_delete(const mem_section_t * const section)
{
	__kernel_assert("__mem_sec_delete - check that the section is valid", section != NULL);
	__mem_free(section->pool, section);
}

uint32_t __mem_sec_get_real_addr(const mem_section_t * const ms)
{
	return ms->real_address;
}

uint32_t __mem_sec_get_virt_addr(const mem_section_t * const ms)
{
	return ms->virt_address;
}

uint32_t __mem_sec_get_size(const mem_section_t * const ms)
{
	return ms->size;
}

mmu_memory_t __mem_sec_get_mem_type(const mem_section_t * const ms)
{
	return ms->memory_type;
}

mmu_privilege_t __mem_sec_get_priv(const mem_section_t * const ms)
{
	return ms->privilege;
}

mmu_access_t __mem_sec_get_access(const mem_section_t * const ms)
{
	return ms->access_rights;
}

const mem_section_t * __mem_sec_get_next(const mem_section_t * const ms)
{
	return ms->next;
}
