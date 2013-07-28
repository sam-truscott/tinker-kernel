/*
 *
 * SOS Source Code
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


#endif /* TGT_MEM_H_ */
