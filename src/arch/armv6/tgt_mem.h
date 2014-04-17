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

typedef struct tgt_mem
{
    uint32_t unused;
} tgt_mem_t;

typedef uint32_t __ppc32_pteg_t;

typedef __ppc32_pteg_t* tgt_pg_tbl_t;

#endif /* TGT_MEM_H_ */
