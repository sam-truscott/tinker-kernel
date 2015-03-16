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

// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0301h/ch06s11s02.html
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0211k/Caceaije.html
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360e/BABIFIHD.html

#define MMU_PAGE_SIZE 	4096

#define NUM_L1_ENTRIES 4096
#define NUM_L2_ENTRIES 256

typedef enum arm_pg_tbl_lvl1_entry
{
	arm_pg_tbl_invalid_lvl1_entry = 0,
	arm_pg_tbl_second_level = 1,
	arm_pg_tbl_section = 2,
	arm_pg_tbl_reserved = 3
} arm_pg_tbl_lvl1_entry_t;

typedef enum arm_pg_tbl_lvl2_entry
{
	arm_pg_tbl_invalid_lvl2_entry = 0,
	arm_pg_tbl_64k_entry = 1,
	arm_pg_tbl_4k_execute_entry = 2,
	arm_pg_tbl_4k_no_execute_entry = 3
} arm_pg_tbl_lvl2_entry_t;

typedef enum arm_pg_tbl_lvl1_ng
{
	arm_pg_tbl_global = 0,
	arm_pg_tbl_process_specific = 1
} arm_pg_tbl_lvl1_ng_t;

typedef enum arm_pg_tbl_lvl1_type
{
	arm_pg_tbl_section_1mb = 0,
	arm_pg_tbl_super_section_16mb = 1
} arm_pg_tbl_lvl1_type_t ;

typedef enum arm_pg_tbl_lvl1_ns
{
	arm_pg_tbl_not_trust_zone = 0,
	arm_pg_tbl_trust_zone = 1
} arm_pg_tbl_lvl1_ns_t;

typedef enum arm_pg_tbl_lvl1_shared {
	arm_pg_tbl_not_shared = 0,
	arm_pg_tbl_shared = 1
} arm_pg_tbl_lvl1_shared_t;

typedef enum arm_pg_tbl_lvl1_apx {
	arm_pg_tbl_apx_off = 0,
	arm_pg_tbl_apx_set = 1
} arm_pg_tbl_lvl1_apx_t;

typedef enum arm_pg_tbl_lvl1_nx {
	arm_pg_tbl_execute = 0,
	arm_pg_tbl_never_execute = 1
} arm_pg_tbl_lvl1_nx_t;

typedef struct tgt_mem_layer_1
{
	uint32_t base : 12;	/* assumes 1mb */	// 32
	arm_pg_tbl_lvl1_ns_t ns : 1;			// 20
	arm_pg_tbl_lvl1_type_t type : 1;		// 19
	arm_pg_tbl_lvl1_ng_t ng : 1;			// 18
	arm_pg_tbl_lvl1_shared_t s : 1;			// 17
	arm_pg_tbl_lvl1_apx_t apx : 1;			// 16
	uint32_t tex : 3;						// 15
	uint32_t ap : 2;						// 12
	const uint32_t p : 1 = 0;				// 10
	uint32_t domain : 4;					//  9
	arm_pg_tbl_lvl1_nx_t nx : 1;			//  5
	bool_t cached : 1;						//  4
	bool_t buffered : 1;					//  3
	arm_pg_tbl_lvl1_entry_t lvl1_type : 2;	//  2
} tgt_mem_layer_1_t;

typedef struct tgt_mem_layer_2
{
	uint32_t base : 20;						// 32
	arm_pg_tbl_lvl1_ng_t ng : 1;			// 12
	arm_pg_tbl_lvl1_shared_t s : 1;			// 11
	arm_pg_tbl_lvl1_apx_t apx : 1;			// 10
	uint32_t sbz : 3;						//  9
	uint32_t ap : 2;						//  6
	bool_t cached : 1;						//  4
	bool_t buffered : 1;					//  3
	arm_pg_tbl_lvl2_entry_t lvl2_type : 2;	//  2
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
