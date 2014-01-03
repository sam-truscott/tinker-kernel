/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_timer.h"
#include "kernel/time/time.h"

#include "arch/tgt_types.h"
#include "object_private.h"
#include "kernel/time/alarm_manager.h"

typedef struct __object_timer_t
{
	__object_internal_t object;
	__mem_pool_info_t * pool;
	sos_timer_callback_t * callback;
	sos_timeout_time_t timeout;
	const void * parameter;
	uint32_t alarm_id;
} __object_timer_internal_t;

__object_timer_t * __obj_cast_timer(__object_t * const o)
{
	__object_timer_t * timer = NULL;
	if (o)
	{
		const __object_timer_t * const tmp = (__object_timer_t*)o;
		if (tmp->object.type == TIMER_OBJ)
		{
			timer = (__object_timer_t*)tmp;
		}
	}
	return timer;
}

object_number_t __obj_timer_get_oid
	(const __object_timer_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

static void _obj_timer_timeout(
		const uint32_t alarm_id,
		const void * const usr_data,
		const uint32_t usr_data_param)
{
	__object_timer_t * const timer = (__object_timer_t*)usr_data;
	if (timer && usr_data_param)
	{
		if (alarm_id == timer->alarm_id)
		{
			// load up the right thread
			// setup the first parameter
			// switch context
			// put this function in user-mode?
			timer->callback(timer->parameter);
		}
	}
}

error_t __obj_create_timer(
		__process_t * const process,
		object_number_t * objectno,
		const uint32_t seconds,
		const uint32_t nanoseconds,
		sos_timer_callback_t * const callback,
		const void* parameter)
{
	error_t result = NO_ERROR;
	if (process && objectno && (seconds || nanoseconds) && callback)
	{
		__object_timer_t * no = NULL;
		__object_table_t * const table = __process_get_object_table(process);
		__mem_pool_info_t * const pool = __process_get_mem_pool(process);
		no = (__object_timer_t*)__mem_alloc(pool, sizeof(__object_timer_t));
		if (no)
		{
			object_number_t objno;
			result = __obj_add_object(table, (__object_t*)no, &objno);
			if (result == NO_ERROR)
			{
				__obj_initialise_object(&no->object, objno, TIMER_OBJ);
				no->timeout.seconds = seconds;
				no->timeout.nanoseconds = nanoseconds;
				no->callback = callback;
				no->parameter = parameter;
				no->pool = pool;
				*objectno = no->object.object_number;
				__time_t timeout = {
						.seconds = seconds,
						.nanoseconds = nanoseconds
				};
				result = __alarm_set_alarm(
						pool,
						&timeout,
						_obj_timer_timeout,
						no,
						sizeof(__object_timer_t*),
						&no->alarm_id);
			}
			else
			{
				__mem_free(pool, no);
			}
		}
		else
		{
			result = OUT_OF_MEMORY;
		}
	}
	else
	{
		result = PARAMETERS_INVALID;
	}
	return result;
}

error_t __obj_cancel_timer(__object_timer_t * const timer)
{
	error_t result = NO_ERROR;
	if (timer)
	{
		if (timer->callback)
		{
			__alarm_unset_alarm(timer->alarm_id);
			timer->alarm_id = 0;
			timer->callback = 0;
		}
		else
		{
			result = ALARM_ALREADY_CANCELLED;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}

error_t __obj_delete_timer(__object_timer_t * const timer)
{
	error_t result = NO_ERROR;
	if (timer)
	{
		if (timer->callback)
		{
			__alarm_unset_alarm(timer->alarm_id);
			timer->alarm_id = 0;
		}
		__mem_free(timer->pool, timer);__alarm_unset_alarm(timer->alarm_id);
		timer->alarm_id = 0;
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}
