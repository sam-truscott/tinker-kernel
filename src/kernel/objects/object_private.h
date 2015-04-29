/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_PRIVATE_H_
#define OBJECT_PRIVATE_H_

#include "arch/tgt_types.h"
#include "kernel/locks/lock.h"

typedef struct object_t
{
	object_number_t object_number;
	object_type_t type;
	lock_t lock;
} object_internal_t;

#endif /* OBJECT_PRIVATE_H_ */
