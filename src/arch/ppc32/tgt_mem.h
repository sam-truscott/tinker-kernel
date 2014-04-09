/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_MEM_H_
#define TGT_MEM_H_

#include "tgt_ints.h"

#define MMU_PAGE_SIZE 	4096
#define MMU_SEG_SIZE 	((256 * 1024) * 1024)
#define MMU_SEG_COUNT	16
#define MMU_MAX_SIZE	(MMU_SEG_SIZE * MMU_SEG_COUNT)

typedef struct tgt_mem
{
	uint32_t segment_ids[MMU_SEG_COUNT];
} tgt_mem_t;

typedef struct __ppc32_pte
{
	uint32_t w0;
	uint32_t w1;
} __ppc32_pte_t;

#define __PPC_MAX_PTE_PER_PTEG 8u

typedef struct __ppc32_pteg
{
	__ppc32_pte_t ptes[__PPC_MAX_PTE_PER_PTEG];
} __ppc32_pteg_t;

typedef __ppc32_pteg_t* tgt_pg_tbl_t;

#endif /* TGT_MEM_H_ */
