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

#include "arch/tgt_types.h"

#define MMU_PAGE_SIZE 	4096

typedef struct tgt_mem
{
    uint32_t unused;
} tgt_mem_t;

typedef void* tgt_pg_tbl_t;

#endif /* TGT_MEM_H_ */
