/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "mem_section.h"

#include "kernel/kernel_assert.h"
#include "kernel/utils/util_memset.h"

typedef struct mem_section_t
{
	mem_section_t *next;
	uint32_t 			real_address;
	uint32_t			virt_address;
	uint32_t 			size;
	mmu_memory_t		memory_type;
	mmu_privilege_t		privilege;
	mmu_access_t		access_rights;
	mem_pool_info_t * pool;
} mem_section_internal_t;

mem_section_t * mem_sec_create(
		mem_pool_info_t * const pool,
		const uint32_t real_addr,
		const uint32_t virt_addr,
		const uint32_t size,
		const mmu_memory_t mem_type,
		const mmu_privilege_t mem_priv,
		const mmu_access_t mem_access)
{
	mem_section_t * const ms = mem_alloc(pool, sizeof(mem_section_t));
	if (ms)
	{
		util_memset(ms, 0, sizeof(mem_section_t));
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

void mem_sec_delete(const mem_section_t * const section)
{
	kernel_assert("mem_sec_delete - check that the section is valid", section != NULL);
	mem_free(section->pool, section);
}

uint32_t mem_sec_get_real_addr(const mem_section_t * const ms)
{
	return ms->real_address;
}

uint32_t mem_sec_get_virt_addr(const mem_section_t * const ms)
{
	return ms->virt_address;
}

uint32_t mem_sec_get_size(const mem_section_t * const ms)
{
	return ms->size;
}

mmu_memory_t mem_sec_get_mem_type(const mem_section_t * const ms)
{
	return ms->memory_type;
}

mmu_privilege_t mem_sec_get_priv(const mem_section_t * const ms)
{
	return ms->privilege;
}

mmu_access_t mem_sec_get_access(const mem_section_t * const ms)
{
	return ms->access_rights;
}

mem_section_t * mem_sec_get_next(const mem_section_t * const ms)
{
	return ms->next;
}

void mem_sec_set_next(mem_section_t * const ms, const mem_section_t * const next)
{
	if (ms)
	{
		ms->next = (mem_section_t*)next;
	}
}
