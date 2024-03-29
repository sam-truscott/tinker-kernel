/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/obj_thread.h"

#include "objects/object_table.h"
#include "objects/object.h"
#include "objects/object_private.h"
#include "console/print_out.h"
#include "memory/memory_manager.h"
#include "process/process_list.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"

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
	scheduler_t * scheduler;
	alarm_manager_t * alarm_manager;
} object_thread_internal_t;

return_t obj_create_thread(
		mem_pool_info_t * const pool,
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager,
		object_table_t * const table,
		const uint32_t thread_id,
		thread_t * const thread,
		object_number_t * const object_no)
{
	object_thread_t * no = NULL;
	return_t result = NO_ERROR;

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
			no->scheduler = scheduler;
			no->thread = thread;
			no->original_priority = thread_get_priority(thread);
			no->priority_inheritance = 0;
			no->alarm_id = 0;
			no->alarm_manager = alarm_manager;

			sch_notify_new_thread(scheduler, thread);

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

object_thread_t * obj_cast_thread(void * const o)
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

return_t obj_exit_thread(object_thread_t * const o)
{
	return_t result = NO_ERROR;

	if (o)
	{
		thread_t * const t = o->thread;

		debug_print(PROCESS, "Objects: proc %d thread %d (%s) is exiting\n", process_get_pid(thread_get_parent(t)), o->tid, thread_get_name(o->thread));

		const thread_state_t state = thread_get_state(t);
		if (state != THREAD_TERMINATED
				&& state != THREAD_NOT_CREATED)
		{
			/* update the reason for the exit and
			 * notify the scheduler so it can do something else*/
			thread_set_state(t, THREAD_TERMINATED);
			sch_notify_exit_thread(o->scheduler, t);
			sch_notify_terminated(o->scheduler, t);

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

return_t obj_get_thread_state(
		object_thread_t * const o,
		thread_state_t * const state)
{
	return_t result = NO_ERROR;


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

return_t obj_set_thread_waiting(
		object_thread_t * const o,
		const object_t * const waiting_on)
{
	return_t result = NO_ERROR;

	if (o)
	{
		const thread_state_t state = thread_get_state(o->thread);
		debug_print(PROCESS, "Process: State of %s is %x\n", thread_get_name(o->thread), state);
		if (state == THREAD_RUNNING
				|| state == THREAD_READY
				|| state == THREAD_PAUSED
				|| state == THREAD_IDLE
				|| state == THREAD_WAITING
				|| state == THREAD_SYSTEM)
		{
			thread_set_state(o->thread, THREAD_WAITING);
			thread_set_waiting_on(o->thread, waiting_on);
			sch_notify_pause_thread(o->scheduler, o->thread);
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

return_t obj_set_thread_ready(object_thread_t * const o)
{
	return_t result = NO_ERROR;

	if (o)
	{
		const thread_state_t state = thread_get_state(o->thread);
		if (state == THREAD_WAITING)
		{
			thread_set_state(o->thread, THREAD_READY);
			thread_set_waiting_on(o->thread, NULL);
			sch_notify_resume_thread(o->scheduler, o->thread);
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

return_t obj_get_thread_priority(
		object_thread_t * const o,
		uint8_t * const priority)
{
	return_t result = NO_ERROR;

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

return_t obj_set_thread_priority(
		object_thread_t * const o,
		const uint8_t priority)
{
	return_t result = NO_ERROR;

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

			sch_notify_change_priority(o->scheduler, o->thread, old_pri);
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

return_t obj_reset_thread_original_priority(object_thread_t * const o)
{
	return_t result = NO_ERROR;

	if (o)
	{
		const uint8_t old_pri = thread_get_priority(o->thread);

		thread_set_priority(o->thread, o->original_priority);
		o->priority_inheritance = 0;
		sch_notify_change_priority(o->scheduler, o->thread, old_pri);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

return_t obj_set_thread_original_priority(object_thread_t * const o)
{
	return_t result = NO_ERROR;

	if (o)
	{
		o->original_priority = thread_get_priority(o->thread);
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

static void obj_thread_sleep_callback(
		alarm_manager_t * const am,
		const uint32_t alarm_id,
		void * const usr_data)
{
	object_thread_t * o = (object_thread_t*)usr_data;
	if (am && o && o->alarm_id == alarm_id)
	{
		thread_set_state(o->thread, THREAD_READY);
		sch_notify_resume_thread(o->scheduler, o->thread);
		alarm_unset_alarm(o->alarm_manager, alarm_id);
	}
}

return_t obj_thread_sleep(object_thread_t * const o, const tinker_time_t * const duration)
{
	return_t result = NO_ERROR;

	if (o && duration)
	{
		alarm_set_alarm(
				o->alarm_manager,
				duration,
				(alarm_call_back*)&obj_thread_sleep_callback,
				o,
				&o->alarm_id);
		thread_set_state(o->thread, THREAD_WAITING);
		thread_set_waiting_on(o->thread, NULL);
		sch_notify_pause_thread(o->scheduler, o->thread);
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}
