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
	o->allocated = OBJECT_ALLOCATED;
	o->type = type;
	o->ref_count = 1;
	o->initialised = OBJECT_INITIALISED;
	__tgt_release_lock(&o->lock);
}

bool __obj_is_initialised(__object_t * const o)
{
	__tgt_acquire_lock(&o->lock);
	bool r = ((o->initialised == OBJECT_INITIALISED) ? true : false);
	__tgt_release_lock(&o->lock);
	return r;
}

bool __obj_is_allocated(__object_t * const o)
{
	__tgt_acquire_lock(&o->lock);
	bool r = ((o->allocated == OBJECT_ALLOCATED) ? true : false);
	__tgt_release_lock(&o->lock);
	return r;
}

uint32_t __obj_get_number(__object_t * const o)
{
	__tgt_acquire_lock(&o->lock);
	uint32_t on = o->object_number;
	__tgt_release_lock(&o->lock);
	return on;
}

__object_type_t __obj_get_type(__object_t * const o)
{
	__tgt_acquire_lock(&o->lock);
	uint32_t ot = o->type;
	__tgt_release_lock(&o->lock);
	return ot;
}

void __obj_increase_ref_count(__object_t * const o)
{
	if ( o )
	{
		o->ref_count++;
	}
}

void __obj_decrease_ref_count(__object_t * const o)
{
	if ( o && o->ref_count )
	{
		o->ref_count--;
	}
}

uint32_t __obj_get_ref_count(const __object_t * const o)
{
	uint32_t c = 0;
	if ( o )
	{
		c = o->ref_count;
	}
	return c;
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
