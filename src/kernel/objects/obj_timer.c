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
#include "config.h"
#include "arch/tgt_types.h"
#include "object_private.h"
#include "obj_thread.h"
#include "kernel/time/alarm_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_manager.h"

typedef struct __object_timer_t
{
	__object_internal_t object;
	__mem_pool_info_t * pool;
	sos_timer_callback_t * callback;
	sos_timeout_time_t timeout;
	const void * parameter;
	uint32_t alarm_id;
	__thread_t * callback_thread;
	__object_thread_t * thread_obj;
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

static void __obj_timer_thread(sos_timer_callback_t * const t, const void * p) __attribute__((section(".utext")));
static void __obj_timer_thread(sos_timer_callback_t * const t, const void * p)
{
	if (t)
	{
		t(p);
	}
}

static void __obj_timer_timeout(
		const uint32_t alarm_id,
		void * const usr_data)
{
	__object_timer_t * const timer = (__object_timer_t*)usr_data;
	if (timer && alarm_id == timer->alarm_id)
	{
		__thread_set_state(timer->callback_thread, THREADY_READY);
		__thread_set_waiting_on(timer->callback_thread, NULL);
		__sch_notify_resume_thread(timer->callback_thread);
		__thread_set_context_param(timer->callback_thread, 0, (uint32_t)timer->callback);
		__thread_set_context_param(timer->callback_thread, 1, (uint32_t)timer->parameter);
		timer->callback = NULL;
		__alarm_unset_alarm(alarm_id);
	}
}

error_t __obj_create_timer(
		__process_t * const process,
		object_number_t * objectno,
		const __priority_t priority,
		const uint32_t seconds,
		const uint32_t nanoseconds,
		sos_timer_callback_t * const callback,
		const void* parameter)
{
	error_t result = NO_ERROR;
	if (process && objectno && (seconds || nanoseconds) && callback)
	{
		__object_table_t * const table = __process_get_object_table(process);
		__mem_pool_info_t * const pool = __process_get_mem_pool(process);
		__object_timer_t * const no = (__object_timer_t*)__mem_alloc(pool, sizeof(__object_timer_t));
		if (no)
		{
			object_number_t objno;
			result = __obj_add_object(table, (__object_t*)no, &objno);
			if (result == NO_ERROR)
			{
				result = __proc_create_thread(
						process,
						"timer",
						(thread_entry_point*)&__obj_timer_thread,
						priority,
						__TIMER_STACK_SIZE,
						THREAD_FLAG_TIMER,
						(__object_t**)&no->thread_obj,
						&no->callback_thread);
				if (result == NO_ERROR)
				{
					__thread_set_state(no->callback_thread, THREAD_WAITING);
					__thread_set_waiting_on(no->callback_thread, (__object_t*)no);
					__obj_initialise_object(&no->object, objno, TIMER_OBJ);
					__sch_notify_pause_thread(no->callback_thread);
					no->timeout.seconds = seconds;
					no->timeout.nanoseconds = nanoseconds;
					no->callback = callback;
					no->parameter = parameter;
					no->pool = pool;
					*objectno = no->object.object_number;
					const __time_t timeout = {
							.seconds = seconds,
							.nanoseconds = nanoseconds
					};
					result = __alarm_set_alarm(
							pool,
							&timeout,
							__obj_timer_timeout,
							no,
							&no->alarm_id);
				}
				else
				{

				}
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
			const object_number_t oid = __obj_thread_get_oid(timer->thread_obj);
			__obj_delete_thread(timer->thread_obj);
			__obj_remove_object(
					__process_get_object_table(
							__thread_get_parent(timer->callback_thread)),
					oid);
		}
		__mem_free(timer->pool, timer);
		timer->alarm_id = 0;
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}
