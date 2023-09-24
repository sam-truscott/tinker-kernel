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

#define MMU_PAGE_SIZE 4096

#define NUM_L1_ENTRIES 4096

#pragma pack(push,1)
typedef struct tgt_mem
{
	uint32_t unused;
} tgt_mem_t;

typedef struct tgt_pg_tbl
{
	uint32_t lvl1_entry[NUM_L1_ENTRIES];
} tgt_pg_tbl_t;
#pragma pack(pop)

#define PAGE_TABLE_SIZE sizeof(tgt_pg_tbl_t)

typedef uint32_t mem_t;

#endif /* TGT_MEM_H_ */
