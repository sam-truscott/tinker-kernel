/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "object.h"
#include "../../arch/tgt.h"

#define OBJECT_INITIALISED 0x55AA
#define OBJECT_ALLOCATED 0xFEED
#define OBJECT_NOT_ALLOCATED 0xDEAD

void __obj_initialise_object(__object_t * o)
{
	__tgt_acquire_lock(&o->lock);
	o->object_number = 0;
	o->allocated = OBJECT_NOT_ALLOCATED;
	o->type = UNKNOWN_OBJ;
	o->ref_count = 1;

	o->specifics.process.pid = 0;

	o->specifics.thread.pid = 0;
	o->specifics.thread.tid = 0;

	o->specifics.pipe.pid_from = 0;
	o->specifics.pipe.pid_to = 0;
	o->specifics.pipe.direction = PIPE_DIRECTION_UNKNOWN;

	o->specifics.semaphore.sem_count = 0;
	o->specifics.semaphore.listeners = NULL;

	o->specifics.shared_memory.real_start = 0;
	o->specifics.shared_memory.virtual_start = 0;
	o->specifics.shared_memory.size = 0;

	o->specifics.clock.time = __ZERO_TIME;

	o->specifics.timer.timeout = __ZERO_TIME;

	o->initialised = OBJECT_INITIALISED;

	__tgt_release_lock(&o->lock);
}

bool __obj_is_initialised(__object_t * o)
{
	__tgt_acquire_lock(&o->lock);
	bool r = ((o->initialised == OBJECT_INITIALISED) ? true : false);
	__tgt_release_lock(&o->lock);
	return r;
}

void __obj_set_allocated(__object_t * o, const bool a )
{
	__tgt_acquire_lock(&o->lock);
	o->allocated = ((a == true) ? OBJECT_ALLOCATED : OBJECT_NOT_ALLOCATED);
	__tgt_release_lock(&o->lock);
}

bool __obj_is_allocated(__object_t * o)
{
	__tgt_acquire_lock(&o->lock);
	bool r = ((o->allocated == OBJECT_ALLOCATED) ? true : false);
	__tgt_release_lock(&o->lock);
	return r;
}

uint32_t __obj_get_number(__object_t * o)
{
	__tgt_acquire_lock(&o->lock);
	uint32_t on = o->object_number;
	__tgt_release_lock(&o->lock);
	return on;
}

void __obj_set_number(__object_t * o, const uint32_t on)
{
	__tgt_acquire_lock(&o->lock);
	o->object_number = on;
	__tgt_release_lock(&o->lock);
}

__object_type_t __obj_get_type(__object_t * o)
{
	__tgt_acquire_lock(&o->lock);
	uint32_t ot = o->type;
	__tgt_release_lock(&o->lock);
	return ot;
}

void __obj_set_type(__object_t * o, const __object_type_t ot)
{
	__tgt_acquire_lock(&o->lock);
	o->type = ot;
	__tgt_release_lock(&o->lock);
}

void __obj_increase_ref_count(__object_t * o)
{
	if ( o )
	{
		o->ref_count++;
	}
}

void __obj_decrease_ref_count(__object_t * o)
{
	if ( o && o->ref_count )
	{
		o->ref_count--;
	}
}

uint32_t __obj_get_ref_count(__object_t * o)
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
void __obj_lock(__object_t * o)
{
	__tgt_acquire_lock(&o->lock);
}

/*
 * MANUAL RELEASE: USE WITH CAUTION
 */
void __obj_release(__object_t * o)
{
	__tgt_release_lock(&o->lock);
}
