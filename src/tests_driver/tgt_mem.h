/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef TESTS_DRIVER_TGT_MEM_H_
#define TESTS_DRIVER_TGT_MEM_H_

#define MEMCPY

#include "arch/tgt_types.h"

#define MMU_PAGE_SIZE 	4096

#pragma pack(push,1)
typedef struct tgt_mem
{
	uint32_t unused;
} tgt_mem_t;

typedef struct tgt_pg_tbl
{
	uint32_t unused;
} tgt_pg_tbl_t;
#pragma pack(pop)

#define PAGE_TABLE_SIZE sizeof(tgt_pg_tbl_t)

#endif /* TESTS_DRIVER_TGT_MEM_H_ */
