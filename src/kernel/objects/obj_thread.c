/*
 *
 * SOS Source Code
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

typedef struct __object_thread_t
{
	__object_internal_t object;
	uint32_t pid;
	uint32_t tid;
	__thread_t * thread;
	/**
	 * This field is used to store the original priority
	 * of a thread during the use-case of priority inheritance
	 */
	priority_t priority_inheritance;
	priority_t original_priority;
	__mem_pool_info_t * pool;
	object_number_t process_obj_no;
} __object_thread_internal_t;

error_t __obj_create_thread(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
	 	const uint32_t process_id,
		const uint32_t thread_id,
		__thread_t * const thread,
		const object_number_t proc_obj_no,
		object_number_t * const object_no)
{
	__object_thread_t * no = NULL;
	error_t result = NO_ERROR;

	if ( table )
	{
		no = __mem_alloc(pool, sizeof(__object_thread_t));
		object_number_t objno;
		result = __obj_add_object(table, (__object_t *)no, &objno);
		if ( result == NO_ERROR )
		{
			__obj_initialise_object(&no->object, objno, THREAD_OBJ);
			__obj_lock(&no->object);
			no->pool = pool;
			no->process_obj_no = proc_obj_no;
			no->pid = process_id;
			no->tid = thread_id;
			no->thread = thread;
			no->original_priority = __thread_get_priority(thread);
			no->priority_inheritance = 0;
			__obj_release(&no->object);

			__sch_notify_new_thread(thread);

			if ( object_no )
			{
				*object_no = no->object.object_number;
			}
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
		if (o->object.initialised == OBJECT_INITIALISED)
		{
			oid = o->object.object_number;
		}
	}
	return oid;
}

object_number_t __obj_thread_get_proc_oid(const __object_thread_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		if (o->object.initialised == OBJECT_INITIALISED)
		{
			oid = o->process_obj_no;
		}
	}
	return oid;
}

__object_thread_t * __obj_cast_thread(__object_t * o)
{
	__object_thread_t * result = NULL;
	if(o)
	{
		const __object_thread_t * const tmp = (const __object_thread_t*)o;
		if (tmp->object.initialised == OBJECT_INITIALISED
			&& tmp->object.type == THREAD_OBJ)
		{
			result = (__object_thread_t*)tmp;
		}
	}
	return result;
}

error_t __obj_pause_thread(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__obj_lock(&o->object);

		const __thread_state_t thread_state = __thread_get_state(o->thread);
		if ( thread_state != thread_not_created &&
				thread_state != thread_terminated )
		{
			__thread_set_state(o->thread, thread_paused);
			__sch_notify_pause_thread(o->thread);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(&o->object);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_resume_thread(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__obj_lock(&o->object);

		const __thread_state_t thread_state = __thread_get_state(o->thread);

		if ( thread_state == thread_paused)
		{
			__thread_set_state(o->thread, thread_ready);
			__sch_notify_resume_thread(o->thread);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(&o->object);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_exit_thread(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(&o->object);
		t = o->thread;

#if defined(__PROCESS_DEBUGGING)
		__debug_print("proc %d thread %d (%s) is exiting\n", o->pid, o->tid, __thread_get_name(o->thread));
#endif

		const __thread_state_t state = __thread_get_state(t);
		if ( state != thread_terminated
				&& state != thread_not_created)
		{
			/* update the reason for the exit and
			 * notify the scheduler so it can do something else*/
			__thread_set_state(t, thread_terminated);
			__sch_notify_exit_thread(t);

			/* free up the stack space used by the thread*/
			__thread_exit(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(&o->object);
		__obj_delete_thread(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_terminate_thread(__object_thread_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = o->thread;

		__obj_lock(&o->object);

		const __thread_state_t state = __thread_get_state(t);

		if ( state != thread_terminated
				&& state != thread_not_created)
		{
			__thread_set_state(t, thread_terminated);
			__sch_notify_exit_thread(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(&o->object);
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


	if ( o && state )
	{
		const __thread_t * t = o->thread;

		__obj_lock(&o->object);
		*state = __thread_get_state(t);
		__obj_release(&o->object);
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

	if ( o )
	{
		__thread_t * t = o->thread;

		__obj_lock(&o->object);

		const __thread_state_t state = __thread_get_state(t);

		if ( state == thread_running )
		{
			__thread_set_state(t, thread_waiting);
			__thread_set_waiting_on(t, waiting_on);
			__sch_notify_pause_thread(t);
		}

		__obj_release(&o->object);
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

	if ( o )
	{
		__thread_t * t = o->thread;

		__obj_lock(&o->object);

		const __thread_state_t state = __thread_get_state(t);

		if ( state == thread_waiting )
		{
			__thread_set_state(t, thread_ready);
			__thread_set_waiting_on(t, NULL);
			__sch_notify_resume_thread(t);
		}

		__obj_release(&o->object);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

priority_t __obj_get_thread_priority_ex(__object_thread_t * const o)
{
	const __thread_t * t = o->thread;
	priority_t p;

	__obj_lock(&o->object);
	p = __thread_get_priority(t);
	__obj_release(&o->object);

	return p;
}

error_t __obj_get_thread_priority(
		__object_thread_t * const o,
		uint8_t * const priority)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		if ( priority )
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

	if ( o )
	{
		if ( priority )
		{
			__thread_t * t = o->thread;

			__obj_lock(&o->object);

			const uint8_t old_pri = __thread_get_priority(t);
			__thread_set_priority(t, priority);

			if ( !o->priority_inheritance )
			{
				o->original_priority = priority;
			}

			__obj_release(&o->object);

			__sch_notify_change_priority(t, old_pri);
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

	if ( o )
	{
		__thread_t * t = o->thread;

		const uint8_t old_pri = __thread_get_priority(t);

		__obj_lock(&o->object);

		__thread_set_priority(t, o->original_priority);
		o->priority_inheritance = 0;
		__sch_notify_change_priority(t, old_pri);

		__obj_release(&o->object);
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

	if ( o )
	{
		__thread_t * t = o->thread;

		__obj_lock(&o->object);
		o->original_priority =
				__thread_get_priority(t);
		o->priority_inheritance = 1;
		__obj_release(&o->object);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_get_thread_original_priority(
		__object_thread_t * const o,
		priority_t * const p)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		*p = __obj_get_thread_original_priority_ex(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

priority_t __obj_get_thread_original_priority_ex(__object_thread_t * const o)
{
	priority_t p;

	__obj_lock(&o->object);
	p = o->original_priority;
	__obj_release(&o->object);

	return p;
}

object_number_t __obj_get_thread_obj_no(const __object_thread_t * const o)
{
	return o->object.object_number;
}

__thread_t * __obj_get_thread(const __object_thread_t * const o)
{
	return o->thread;
}
