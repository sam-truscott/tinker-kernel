/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_PRIVATE_H_
#define OBJECT_PRIVATE_H_

#include "arch/tgt_types.h"

typedef struct __object_t
{
	object_number_t object_number;
	__object_type_t type;
} __object_internal_t;

#endif /* OBJECT_PRIVATE_H_ */
