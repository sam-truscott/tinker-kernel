/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_TYPES_H_
#define TGT_TYPES_H_

#include "tgt_ints.h"
#include "tgt_mem.h"
#include "tgt_io.h"

typedef enum bool
{
	false = 0,
	true = 1
} bool_t;

#define NULL ((void*)0)

typedef volatile uint8_t spinlock_t;
#define LOCK_ON 0xDE
#define LOCK_OFF 0xED

#define SWAP_32(V) \
	(V & 0x000000FF) << 24 | \
	(V & 0x0000FF00) << 8  | \
	(V & 0x00FF0000) >> 8  | \
	(V & 0xFF000000) >> 24

typedef struct tgt_context_t tgt_context_t;

#endif /* TGT_TYPES_H_ */
