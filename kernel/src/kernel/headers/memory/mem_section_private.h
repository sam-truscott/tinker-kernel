#ifndef MEM_SECTION_PRIVATE_H_
#define MEM_SECTION_PRIVATE_H_

#include "mem_pool.h"

typedef struct mem_section_t mem_section_t;

typedef enum
{
	MMU_DEVICE_MEMORY		 	= 0,
	MMU_RANDOM_ACCESS_MEMORY	= 1
} mmu_memory_t;

typedef enum
{
	MMU_NO_PRIVILEGE 	= 0,
	MMU_USER_ACCESS		= 1,
	MMU_KERNEL_ACCESS	= 2,
	MMU_ALL_ACCESS		= 3
} mmu_privilege_t;

typedef enum
{
	MMU_NO_ACCESS	= 0,
	MMU_READ_ONLY	= 1,
	MMU_READ_WRITE	= 2
} mmu_access_t;

typedef struct mem_section_t
{
	mem_section_t   * next;
	mem_t 			real_address;
	mem_t			virt_address;
	mem_t 			size;
	mmu_memory_t	memory_type;
	mmu_privilege_t	privilege;
	mmu_access_t	access_rights;
	mem_pool_info_t * pool;
} mem_section_internal_t;

void mem_sec_initialise(
		mem_section_t * const sec,
		mem_pool_info_t * const pool,
		const mem_t real_addr,
		const mem_t virt_addr,
		const mem_t size,
		const mmu_memory_t mem_type,
		const mmu_privilege_t mem_priv,
		const mmu_access_t mem_access);

#endif /* MEM_SECTION_PRIVATE_H_ */
