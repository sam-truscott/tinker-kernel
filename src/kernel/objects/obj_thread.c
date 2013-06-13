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
#include "kernel/memory/memory_manager.h"
#include "kernel/process/process_manager.h"
#include "kernel/scheduler/scheduler.h"

error_t __obj_create_thread(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
	 	const uint32_t process_id,
		const uint32_t thread_id,
		__thread_t * const thread,
		__object_t ** object)
{
	__object_t * no = NULL;
	error_t result = NO_ERROR;

	if ( table )
	{
		if ( (result = __obj_allocate_next_free_object(pool, table, &no)) == NO_ERROR )
		{
			__obj_set_type(no, THREAD_OBJ);
			__obj_set_allocated(no, true);
			__obj_lock(no);
			no->specifics.thread.pid = process_id;
			no->specifics.thread.tid = thread_id;
			no->specifics.thread.thread = thread;
			no->specifics.thread.original_priority = thread->priority;
			no->specifics.thread.priority_inheritance = 0;
			__obj_release(no);

			__sch_notify_new_thread(thread);

			if ( object )
			{
				*object = no;
			}
		}
	}
	else
	{
		result = PARAMETERS_OUT_OF_RANGE;
	}

	return result;
}

error_t __obj_pause_thread(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;

		if ( t->state != thread_not_created &&
				t->state != thread_terminated )
		{
			t->state = thread_paused;
			__sch_notify_pause_thread(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_resume_thread(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;

		if ( t->state == thread_paused)
		{
			t->state = thread_ready;
			__sch_notify_resume_thread(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_exit_thread(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;

		if ( t && t->state != thread_terminated
				&& t->state != thread_not_created)
		{
                        /* update the reason for the exit and
                         * notify the scheduler so it can do something else*/
			t->state = thread_terminated;
			__sch_notify_exit_thread(t);
                        
                        /* free up the stack space used by the thread*/
                        __mem_free(t->parent->memory_pool, t->stack);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_terminate_thread(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;

		if ( t && t->state != thread_terminated
				&& t->state != thread_not_created)
		{
			t->state = thread_terminated;
			__sch_notify_exit_thread(t);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_get_thread_state(
		__object_t * const o,
		__thread_state_t * const state)
{
	error_t result = NO_ERROR;


	if ( o && state )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;
		*state = t->state;

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_set_thread_waiting(__object_t * const o, const __object_t * const waiting_on)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;

		if ( t && t->state == thread_running )
		{
			t->state = thread_waiting;
			t->waiting_on = waiting_on;
			__sch_notify_pause_thread(t);
		}

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_set_thread_ready(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__thread_t * t = NULL;

		__obj_lock(o);

		t = o->specifics.thread.thread;

		if ( t && t->state == thread_waiting )
		{
			t->state = thread_ready;
			t->waiting_on = NULL;
			__sch_notify_resume_thread(t);
		}

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_get_thread_priority(
		__object_t * const o,
		uint8_t * const priority)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		if ( priority )
		{
			__obj_lock(o);

			*priority = o->specifics.thread.thread->priority;

			__obj_release(o);
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
		__object_t * const o,
		const uint8_t priority)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		if ( priority )
		{
			uint8_t old_pri = o->specifics.thread.thread->priority;
			__obj_lock(o);

			o->specifics.thread.thread->priority = priority;

			if ( !o->specifics.thread.priority_inheritance )
			{
				o->specifics.thread.original_priority = priority;
			}

			__obj_release(o);

			__sch_notify_change_priority(o->specifics.thread.thread, old_pri);
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

error_t __obj_reset_thread_original_priority(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		uint8_t old_pri = o->specifics.thread.thread->priority;
		__obj_lock(o);

		o->specifics.thread.thread->priority =
				o->specifics.thread.original_priority;
		o->specifics.thread.priority_inheritance = 0;

		__sch_notify_change_priority(o->specifics.thread.thread, old_pri);

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}

error_t __obj_set_thread_original_priority(__object_t * const o)
{
	error_t result = NO_ERROR;

	if ( o )
	{
		__obj_lock(o);

		o->specifics.thread.original_priority =
				o->specifics.thread.thread->priority;
		o->specifics.thread.priority_inheritance = 1;

		__obj_release(o);
	}
	else
	{
		result = INVALID_OBJECT;
	}

	return result;
}
