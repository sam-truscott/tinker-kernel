/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arch/target_types.h"

void __tgt_acquire_lock(__spinlock_t * lock)
{
	while ( ! *lock )
	{
		*lock = LOCK_ON;
	}
}
#pragma weak __tgt_acquire_lock

void __tgt_release_lock(__spinlock_t * lock)
{
	*lock = LOCK_OFF;
}
#pragma weak __tgt_release_lock
