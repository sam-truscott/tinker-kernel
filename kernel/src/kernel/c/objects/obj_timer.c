/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/obj_timer.h"

#include "time/time.h"
#include "config.h"
#include "tgt_types.h"
#include "objects/object_private.h"
#include "objects/obj_thread.h"
#include "time/alarm_manager.h"
#include "scheduler/scheduler.h"
#include "process/process_list.h"
#include "console/print_out.h"

typedef struct object_timer_t
{
	object_internal_t object;
	mem_pool_info_t * pool;
	scheduler_t * scheduler;
	alarm_manager_t * alarm_manager;
	tinker_timer_callback_t * callback;
	tinker_timeout_time_t timeout;
	const void * parameter;
	uint32_t alarm_id;
	thread_t * callback_thread;
	object_thread_t * thread_obj;
} object_timer_internal_t;

object_timer_t * obj_cast_timer(void * const o)
{
	object_timer_t * timer = NULL;
	if (o)
	{
		const object_timer_t * const tmp = (object_timer_t*)o;
		if (tmp->object.type == TIMER_OBJ)
		{
			timer = (object_timer_t*)tmp;
		}
	}
	return timer;
}

object_number_t obj_timer_get_oid
	(const object_timer_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

static void obj_timer_thread(tinker_timer_callback_t * const t, const void * p) __attribute__((section(".api")));
static void obj_timer_thread(tinker_timer_callback_t * const t, const void * p)
{
	if (is_debug_enabled(TIMER))
	{
		debug_prints(TIMER, "Timer: Callback thread\n");
	}
	if (t)
	{
		t(p);
	}
}

static void obj_timer_timeout(
		alarm_manager_t * const am,
		const uint32_t alarm_id,
		void * const usr_data)
{
	object_timer_t * const timer = (object_timer_t*)usr_data;
	if (is_debug_enabled(TIMER))
	{
		debug_print(TIMER, "Timer: My alarm is id %d, fired alarm is %d\n", timer->alarm_id, alarm_id);
	}
	if (timer && (alarm_id == timer->alarm_id))
	{
		thread_set_state(timer->callback_thread, THREAD_READY);
		thread_set_waiting_on(timer->callback_thread, NULL);
		if (is_debug_enabled(TIMER))
		{
			debug_print(TIMER, "Timer: Resuming waiting thread\n", timer->alarm_id, alarm_id);
		}
		sch_notify_resume_thread(timer->scheduler, timer->callback_thread);
		thread_set_context_param(timer->callback_thread, 0, (uint32_t)timer->callback);
		thread_set_context_param(timer->callback_thread, 1, (uint32_t)timer->parameter);
		timer->callback = NULL;
		alarm_unset_alarm(am, alarm_id);
	}
}

return_t obj_create_timer(
		scheduler_t * const scheduler,
		alarm_manager_t * const alarm_manager,
		process_t * const process,
		object_number_t * objectno,
		const priority_t priority,
		const uint32_t seconds,
		const uint32_t nanoseconds,
		tinker_timer_callback_t * const callback,
		const void* parameter)
{
	return_t result = NO_ERROR;
	if (process && objectno && (seconds || nanoseconds) && callback)
	{
		object_table_t * const table = process_get_object_table(process);
		mem_pool_info_t * const pool = process_get_mem_pool(process);
		object_timer_t * const no = (object_timer_t*)mem_alloc(pool, sizeof(object_timer_t));
		if (no)
		{
			object_number_t objno;
			result = obj_add_object(table, (object_t*)no, &objno);
			if (result == NO_ERROR)
			{
				result = proc_create_thread(
						process,
						"timer",
						(thread_entry_point*)&obj_timer_thread,
						priority,
						TIMER_STACK_SIZE,
						THREAD_FLAG_TIMER,
						(object_t**)&no->thread_obj,
						&no->callback_thread);
				if (result == NO_ERROR)
				{
					thread_set_state(no->callback_thread, THREAD_WAITING);
					thread_set_waiting_on(no->callback_thread, (object_t*)no);
					obj_initialise_object(&no->object, objno, TIMER_OBJ);
					sch_notify_pause_thread(scheduler, no->callback_thread);
					no->timeout.seconds = seconds;
					no->timeout.nanoseconds = nanoseconds;
					no->callback = callback;
					no->parameter = parameter;
					no->pool = pool;
					no->scheduler = scheduler;
					no->alarm_manager = alarm_manager;
					*objectno = no->object.object_number;
					const tinker_time_t timeout = {
							.seconds = seconds,
							.nanoseconds = nanoseconds
					};
					result = alarm_set_alarm(
							alarm_manager,
							&timeout,
							obj_timer_timeout,
							no,
							&no->alarm_id);
				}
				else
				{

				}
			}
			else
			{
				mem_free(pool, no);
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

static void obj_timer_delete_thread(object_timer_t * const timer)
{
	thread_t * const thread = obj_get_thread(timer->thread_obj);
	const thread_state_t state = thread_get_state(thread);
	const object_number_t oid = obj_thread_get_oid(timer->thread_obj);

	if (state != THREAD_TERMINATED && state != THREAD_NOT_CREATED)
	{
		sch_notify_exit_thread(timer->scheduler, thread);
	}
	process_thread_exit(thread_get_parent(thread), thread);
	obj_exit_thread(timer->thread_obj);

	obj_remove_object(
			process_get_object_table(
					thread_get_parent(timer->callback_thread)),
			oid);
}

return_t obj_cancel_timer(object_timer_t * const timer)
{
	return_t result = NO_ERROR;
	if (timer)
	{
		if (timer->callback)
		{
			alarm_unset_alarm(timer->alarm_manager, timer->alarm_id);
			timer->alarm_id = 0;
			timer->callback = 0;
			obj_timer_delete_thread(timer);
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

return_t obj_delete_timer(object_timer_t * const timer)
{
	return_t result = NO_ERROR;
	if (timer)
	{
		if (timer->callback)
		{
			alarm_unset_alarm(timer->alarm_manager, timer->alarm_id);
			timer->alarm_id = 0;
			obj_timer_delete_thread(timer);
		}
		mem_free(timer->pool, timer);
		timer->alarm_id = 0;
	}
	else
	{
		result = PARAMETERS_NULL;
	}
	return result;
}
