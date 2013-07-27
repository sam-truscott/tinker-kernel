/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TARGET_TYPES_H_
#define TARGET_TYPES_H_

#include "tgt_types.h"

typedef volatile uint8_t __spinlock_t;
#define LOCK_ON 0xDE
#define LOCK_OFF 0xED

#define SWAP_32(V) \
	(V & 0x000000FF) << 24 | \
	(V & 0x0000FF00) << 8  | \
	(V & 0x00FF0000) >> 8  | \
	(V & 0xFF000000) >> 24

#endif /* TARGET_TYPES_H_ */
