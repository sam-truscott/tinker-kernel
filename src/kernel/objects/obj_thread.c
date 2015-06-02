/*
 *
 * TINKER Source Code
 * 
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
#include "kernel/process/process_list.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"

typedef struct object_thread_t
{
	object_internal_t object;
	uint32_t tid;
	thread_t * thread;
	/**
	 * This field is used to store the original priority
	 * of a thread during the use-case of priority inheritance
	 */
	priority_t priority_inheritance;
	priority_t original_priority;
	mem_pool_info_t * pool;
	uint32_t alarm_id;
} object_thread_internal_t;

error_t obj_create_thread(
		mem_pool_info_t * const pool,
		object_table_t * const table,
		const uint32_t thread_id,
		thread_t * const thread,
		object_number_t * const object_no)
{
	object_thread_t * no = NULL;
	error_t result = NO_ERROR;

	if ( table )
	{
		no = mem_alloc(pool, sizeof(object_thread_t));
		object_number_t objno;
		result = obj_add_object(table, (object_t *)no, &objno);
		if (result == NO_ERROR)
		{
			obj_initialise_object(&no->object, objno, THREAD_OBJ);
			no->pool = pool;
			no->tid = thread_id;
			no->thread = thread;
			no->original_priority = thread_get_priority(thread);
			no->priority_inheritance = 0;
			no->alarm_id = 0;

			sch_notify_new_thread(thread);

			if ( object_no )
			{
				*object_no = no->object.object_number;
			}
		}
		else
		{
			mem_free(pool, no);
		}
	}
	else
	{
		result = PARAMETERS_OUT_OF_RANGE;
	}

	return result;
}

void obj_delete_thread(object_thread_t * const thread)
{
	if (thread)
	{
		mem_free(thread->pool, thread);
	}
}

object_number_t obj_thread_get_oid
	(const object_thread_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

object_thread_t * obj_cast_thread(object_t * o)
{
	object_thread_t * result = NULL;
	if(o)
	{
		const object_thread_t * const tmp = (const object_thread_t*)o;
		if (tmp->object.type == THREAD_OBJ)
		{
			result = (object_thread_t*)tmp;
		}
	}
	return result;
}

error_t obj_exit_thread(object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		thread_t * const t = o->thread;

#if defined(PROCESS_DEBUGGING)
		const uint32_t pid = process_get_pid(thread_get_parent(t));
		debug_print("Objects: proc %d thread %d (%s) is exiting\n", pid, o->tid, thread_get_name(o->thread));
#endif

		const thread_state_t state = thread_get_state(t);
		if (state != THREAD_TERMINATED
				&& state != THREAD_NOT_CREATED)
		{
			/* update the reason for the exit and
			 * notify the scheduler so it can do something else*/
			thread_set_state(t, THREAD_TERMINATED);
			sch_notify_exit_thread(t);

			/* free up the stack space used by the thread*/
			thread_exit(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		obj_delete_thread(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t obj_get_thread_state(
		object_thread_t * const o,
		thread_state_t * const state)
{
	error_t result = NO_ERROR;


	if (o && state)
	{
		const thread_t * t = o->thread;
		*state = thread_get_state(t);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t obj_set_thread_waiting(
		object_thread_t * const o,
		const object_t * const waiting_on)
{
	error_t result = NO_ERROR;

	if (o)
	{
		const thread_state_t state = thread_get_state(o->thread);
		if (state == THREAD_RUNNING)
		{
			thread_set_state(o->thread, THREAD_WAITING);
			thread_set_waiting_on(o->thread, waiting_on);
			sch_notify_pause_thread(o->thread);
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

bool_t obj_thread_is_waiting_on(
		const object_thread_t * const o,
		const object_t * const waiting_on)
{
	bool_t is_waiting;
	if (o)
	{
		is_waiting = (thread_get_waiting_on(o->thread) == waiting_on);
	}
	else
	{
		is_waiting = false;
	}
	return is_waiting;
}

error_t obj_set_thread_ready(object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		const thread_state_t state = thread_get_state(o->thread);
		if (state == THREAD_WAITING)
		{
			thread_set_state(o->thread, THREADY_READY);
			thread_set_waiting_on(o->thread, NULL);
			sch_notify_resume_thread(o->thread);
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

priority_t obj_get_thread_priority_ex(object_thread_t * const o)
{
	priority_t p  = 0;
	if (o)
	{
		p = thread_get_priority(o->thread);
	}
	return p;
}

error_t obj_get_thread_priority(
		object_thread_t * const o,
		uint8_t * const priority)
{
	error_t result = NO_ERROR;

	if (o)
	{
		if (priority)
		{
			*priority = obj_get_thread_priority_ex(o);
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

error_t obj_set_thread_priority(
		object_thread_t * const o,
		const uint8_t priority)
{
	error_t result = NO_ERROR;

	if (o)
	{
		if (priority)
		{
			const uint8_t old_pri = thread_get_priority(o->thread);
			thread_set_priority(o->thread, priority);

			if (!o->priority_inheritance)
			{
				o->original_priority = priority;
			}

			sch_notify_change_priority(o->thread, old_pri);
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

error_t obj_reset_thread_original_priority(object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		const uint8_t old_pri = thread_get_priority(o->thread);

		thread_set_priority(o->thread, o->original_priority);
		o->priority_inheritance = 0;
		sch_notify_change_priority(o->thread, old_pri);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t obj_set_thread_original_priority(object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if (o)
	{
		o->original_priority =
				thread_get_priority(o->thread);
		o->priority_inheritance = 1;
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

priority_t obj_get_thread_original_priority_ex(object_thread_t * const o)
{
	priority_t p = 0;

	if (o)
	{
		p = o->original_priority;
	}

	return p;
}

thread_t * obj_get_thread(const object_thread_t * const o)
{
	thread_t * t = NULL;
	if (o)
	{
		t = o->thread;
	}
	return t;
}

static void obj_thread_sleep_callback(const uint32_t alarm_id, object_thread_t * const o)
{
	if (o && o->alarm_id == alarm_id)
	{
		thread_set_state(o->thread, THREADY_READY);
		sch_notify_resume_thread(o->thread);
		alarm_unset_alarm(alarm_id);
	}
}

error_t obj_thread_sleep(object_thread_t * const o, const tinker_time_t * const duration)
{
	error_t result = NO_ERROR;

	if (o && duration)
	{
		alarm_set_alarm(
				process_get_mem_pool(thread_get_parent(o->thread)),
				duration,
				(alarm_call_back*)&obj_thread_sleep_callback,
				o,
				&o->alarm_id);
		thread_set_state(o->thread, THREAD_WAITING);
		thread_set_waiting_on(o->thread, NULL);
		sch_notify_pause_thread(o->thread);
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}
