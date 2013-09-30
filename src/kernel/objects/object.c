/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "object.h"

#include "object_private.h"
#include "arch/tgt.h"

void __obj_initialise_object(
		__object_t * const o,
		const object_number_t id,
		const __object_type_t type)
{
	o->object_number = id;
	o->type = type;
}

__object_type_t __obj_get_type(const __object_t * const o)
{
	__object_type_t type = UNKNOWN_OBJ;
	if (o)
	{
		type = o->type;
	}
	return type;
}

uint32_t __obj_get_number(const __object_t * const o)
{
	uint32_t no = 0;
	if (o)
	{
		no = o->object_number;
	}
	return no;
}

