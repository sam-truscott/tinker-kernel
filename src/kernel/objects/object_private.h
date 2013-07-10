/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_PRIVATE_H_
#define OBJECT_PRIVATE_H_

#include "arch/target_types.h"

#define OBJECT_INITIALISED 0x55AA
#define OBJECT_ALLOCATED 0xFEED
#define OBJECT_NOT_ALLOCATED 0xDEAD

typedef struct __object_t
{
	object_number_t object_number;
	uint16_t initialised;
	__spinlock_t lock;
	uint16_t allocated;
	__object_type_t type;
	uint32_t ref_count;
} __object_internal_t;

#endif /* OBJECT_PRIVATE_H_ */
