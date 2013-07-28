/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_shared_mem.h"
#include "arch/tgt_types.h"

typedef struct __object_shm_t
{
	uint32_t real_start;
	uint32_t virtual_start;
	uint32_t size;
} __object_shm_internal_t;
