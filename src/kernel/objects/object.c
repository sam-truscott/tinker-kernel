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
	__tgt_acquire_lock(&o->lock);
	o->object_number = id;
	o->type = type;
	__tgt_release_lock(&o->lock);
}

uint32_t __obj_get_number(__object_t * const o)
{
	__tgt_acquire_lock(&o->lock);
	uint32_t on = o->object_number;
	__tgt_release_lock(&o->lock);
	return on;
}

/*
 * MANUAL LOCK: USE WITH CAUTION
 */
void __obj_lock(__object_t * const o)
{
	__tgt_acquire_lock(&o->lock);
}

/*
 * MANUAL RELEASE: USE WITH CAUTION
 */
void __obj_release(__object_t * const o)
{
	__tgt_release_lock(&o->lock);
}
