/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_MEM_H_
#define TGT_MEM_H_

#include "tgt_types.h"

#define PAGE_TABLE_SIZE (1 * 1024 * 1024)
#define PAGE_TABLE_ALIGNMENT (64 * 1024)
#define MMU_PAGE_SIZE 	4096
#define MMU_SEG_SIZE 	((256 * 1024) * 1024)
#define MMU_SEG_COUNT	16
#define MMU_MAX_SIZE	(MMU_SEG_SIZE * MMU_SEG_COUNT)

typedef struct tgt_mem
{
	uint32_t segment_ids[MMU_SEG_COUNT];
} tgt_mem_t;

typedef struct ppc32_pte
{
	uint32_t w0;
	uint32_t w1;
} ppc32_pte_t;

#define PPC_MAX_PTE_PER_PTEG 8u

typedef struct ppc32_pteg
{
	ppc32_pte_t ptes[PPC_MAX_PTE_PER_PTEG];
} ppc32_pteg_t;

typedef ppc32_pteg_t* tgt_pg_tbl_t;

#endif /* TGT_MEM_H_ */
