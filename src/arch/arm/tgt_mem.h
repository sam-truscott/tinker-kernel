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

#define NUM_L1_ENTRIES 4096
#define NUM_L2_ENTRIES 256

typedef struct tgt_mem_layer_1
{
	uint32_t base : 22;
	uint32_t perm : 2;
	uint32_t dom  : 4;
	uint32_t cache_buff : 2;
	uint32_t type : 2;
} tgt_mem_layer_1_t;

typedef struct tgt_mem_layer_2
{
	uint32_t base : 20;
	uint32_t perm : 8;
	uint32_t cache_buff : 2;
	uint32_t validity : 2;
} tgt_mem_layer_2_t;

typedef struct l2 {
	tgt_mem_layer_2_t l2[NUM_L2_ENTRIES];
} l2_t;

typedef struct tgt_mem
{
	tgt_mem_layer_1_t l1[NUM_L1_ENTRIES];
} tgt_mem_t;

typedef void* tgt_pg_tbl_t;

#endif /* TGT_MEM_H_ */
