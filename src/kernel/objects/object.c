/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "object.h"

#include "object_private.h"
#include "arch/tgt.h"

void obj_initialise_object(
		object_t * const o,
		const object_number_t id,
		const object_type_t type)
{
	o->object_number = id;
	o->type = type;
	lock_init(&o->lock);
}

object_type_t obj_get_type(const object_t * const o)
{
	object_type_t type = UNKNOWN_OBJ;
	if (o)
	{
		type = o->type;
	}
	return type;
}

uint32_t obj_get_number(const object_t * const o)
{
	uint32_t no = 0;
	if (o)
	{
		no = o->object_number;
	}
	return no;
}

void obj_lock(object_t * const o)
{
	if (o)
	{
#if defined (LOCK_DEBUGGING)
		debug_print("locking object %d\n", o->object_number);
#endif
		lock(&o->lock);
#if defined (LOCK_DEBUGGING)
		debug_print("locked object %d\n", o->object_number);
#endif
	}
}

void obj_unlock(object_t * const o)
{
	if (o)
	{
#if defined (LOCK_DEBUGGING)
		debug_print("unlocking object %d\n", o->object_number);
#endif
		unlock(&o->lock);
#if defined (LOCK_DEBUGGING)
		debug_print("unlocked object %d\n", o->object_number);
#endif
	}
}

