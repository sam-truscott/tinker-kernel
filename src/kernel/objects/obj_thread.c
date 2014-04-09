/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_thread.h"

#include "object_table.h"
#include "object.h"
#include "object_private.h"
#include "kernel/console/print_out.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/process/process_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"

typedef struct __object_thread_t
{
	__object_internal_t object;
	uint32_t tid;
	__thread_t * thread;
	/**
	 * This field is used to store the original priority
	 * of a thread during the use-case of priority inheritance
	 */
	__priority_t priority_inheritance;
	__priority_t original_priority;
	__mem_pool_info_t * pool;
	uint32_t alarm_id;
} __object_thread_internal_t;

error_t __obj_create_thread(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		const uint32_t thread_id,
		__thread_t * const thread,
		object_number_t * const object_no)
{
	__object_thread_t * no = NULL;
	error_t result = NO_ERROR;

	if ( table )
	{
		no = __mem_alloc(pool, sizeof(__object_thread_t));
		object_number_t objno;
		result = __obj_add_object(table, (__object_t *)no, &objno);
		if (result == NO_ERROR)
		{
			__obj_initialise_object(&no->object, objno, THREAD_OBJ);
			no->pool = pool;
			no->tid = thread_id;
			no->thread = thread;
			no->original_priority = __thread_get_priority(thread);
			no->priority_inheritance = 0;
			no->alarm_id = 0;

			__sch_notify_new_thread(thread);

			if ( object_no )
			{
				*object_no = no->object.object_number;
			}
		}
		else
		{
			__mem_free(pool, no);
		}
	}
	else
	{
		result = PARAMETERS_OUT_OF_RANGE;
	}

	return result;
}

void __obj_delete_thread(__object_thread_t * const thread)
{
	if (thread)
	{
		__mem_free(thread->pool, thread);
	}
}

object_number_t __obj_thread_get_oid
	(const __object_thread_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

__object_thread_t * __obj_cast_thread(__object_t * o)
{
	__object_thread_t * result = NULL;
	if(o)
	{
		const __object_thread_t * const tmp = (const __object_thread_t*)o;
		if (tmp->object.type == THREAD_OBJ)
		{
			result = (__object_thread_t*)tmp;
		}
	}
	return result;
}

error_t __obj_exit_thread(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		__thread_t * const t = o->thread;

#if defined(__PROCESS_DEBUGGING)
		const uint32_t pid = __process_get_pid(__thread_get_parent(t));
		__debug_print("proc %d thread %d (%s) is exiting\n", pid, o->tid, __thread_get_name(o->thread));
#endif

		const __thread_state_t state = __thread_get_state(t);
		if (state != THREAD_TERMINATED
				&& state != THREAD_NOT_CREATED)
		{
			/* update the reason for the exit and
			 * notify the scheduler so it can do something else*/
			__thread_set_state(t, THREAD_TERMINATED);
			__sch_notify_exit_thread(t);

			/* free up the stack space used by the thread*/
			__thread_exit(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_delete_thread(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_get_thread_state(
		__object_thread_t * const o,
		__thread_state_t * const state)
{
	error_t result = NO_ERROR;


	if (o && state)
	{
		const __thread_t * t = o->thread;
		*state = __thread_get_state(t);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_set_thread_waiting(
		__object_thread_t * const o,
		const __object_t * const waiting_on)
{
	error_t result = NO_ERROR;

	if (o)
	{
		const __thread_state_t state = __thread_get_state(o->thread);
		if (state == THREAD_RUNNING)
		{
			__thread_set_state(o->thread, THREAD_WAITING);
			__thread_set_waiting_on(o->thread, waiting_on);
			__sch_notify_pause_thread(o->thread);
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

bool_t __obj_thread_is_waiting_on(
		const __object_thread_t * const o,
		const __object_t * const waiting_on)
{
	return __thread_get_waiting_on(o->thread) == waiting_on;
}

error_t __obj_set_thread_ready(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		const __thread_state_t state = __thread_get_state(o->thread);
		if (state == THREAD_WAITING)
		{
			__thread_set_state(o->thread, THREADY_READY);
			__thread_set_waiting_on(o->thread, NULL);
			__sch_notify_resume_thread(o->thread);
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

__priority_t __obj_get_thread_priority_ex(__object_thread_t * const o)
{
	__priority_t p  = 0;
	if (o)
	{
		p = __thread_get_priority(o->thread);
	}
	return p;
}

error_t __obj_get_thread_priority(
		__object_thread_t * const o,
		uint8_t * const priority)
{
	error_t result = NO_ERROR;

	if (o)
	{
		if (priority)
		{
			*priority = __obj_get_thread_priority_ex(o);
		}
		else
		{
			result = PARAMETERS_INVALID;
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_set_thread_priority(
		__object_thread_t * const o,
		const uint8_t priority)
{
	error_t result = NO_ERROR;

	if (o)
	{
		if (priority)
		{
			const uint8_t old_pri = __thread_get_priority(o->thread);
			__thread_set_priority(o->thread, priority);

			if (!o->priority_inheritance)
			{
				o->original_priority = priority;
			}

			__sch_notify_change_priority(o->thread, old_pri);
		}
		else
		{
			result = PARAMETERS_INVALID;
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_reset_thread_original_priority(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		const uint8_t old_pri = __thread_get_priority(o->thread);

		__thread_set_priority(o->thread, o->original_priority);
		o->priority_inheritance = 0;
		__sch_notify_change_priority(o->thread, old_pri);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_set_thread_original_priority(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		o->original_priority =
				__thread_get_priority(o->thread);
		o->priority_inheritance = 1;
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

__priority_t __obj_get_thread_original_priority_ex(__object_thread_t * const o)
{
	__priority_t p = 0;

	if (o)
	{
		p = o->original_priority;
	}

	return p;
}

__thread_t * __obj_get_thread(const __object_thread_t * const o)
{
	__thread_t * t = NULL;
	if (o)
	{
		t = o->thread;
	}
	return t;
}

static void __obj_thread_sleep_callback(const uint32_t alarm_id, __object_thread_t * const o)
{
	if (o && o->alarm_id == alarm_id)
	{
		__thread_set_state(o->thread, THREADY_READY);
		__sch_notify_resume_thread(o->thread);
		__alarm_unset_alarm(alarm_id);
	}
}

error_t __obj_thread_sleep(__object_thread_t * const o, const tinker_time_t * const duration)
{
	error_t result = NO_ERROR;

	if (o && duration)
	{
		__alarm_set_alarm(
				__process_get_mem_pool(__thread_get_parent(o->thread)),
				duration,
				(__alarm_call_back*)&__obj_thread_sleep_callback,
				o,
				&o->alarm_id);
		__thread_set_state(o->thread, THREAD_WAITING);
		__thread_set_waiting_on(o->thread, NULL);
		__sch_notify_pause_thread(o->thread);
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}
