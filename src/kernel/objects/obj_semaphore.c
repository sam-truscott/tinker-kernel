/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_semaphore.h"

#include "object.h"
#include "object_private.h"
#include "obj_thread.h"
#include "object_table.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/utils/collections/unbounded_queue.h"

UNBOUNDED_QUEUE_TYPE(thread_obj_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC(static,thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY(static,thread_obj_queue_t, __object_thread_t*)

typedef struct __object_sema_t
{
	__object_internal_t object;
	uint32_t sem_count;
	uint32_t sem_alloc;
	thread_obj_queue_t * listeners;
	thread_obj_queue_t * owners;
	priority_t highest_priority;
	__mem_pool_info_t * pool;
} __object_sema_internal_t;

static void __obj_push_semaphore_listener(
		thread_obj_queue_t * const list,
		__object_thread_t * const object);

static void __obj_notify_semaphore_listener(
		__object_sema_t * const semaphore,
		thread_obj_queue_t * const list);

__object_sema_t * __obj_cast_semaphore(__object_t * o)
{
	__object_sema_t * result = NULL;
	if(o)
	{
		const __object_sema_t * const tmp = (const __object_sema_t*)o;
		if (tmp->object.initialised == OBJECT_INITIALISED
			&& tmp->object.type == SEMAPHORE_OBJ)
		{
			result = (__object_sema_t*)tmp;
		}
	}
	return result;
}

object_number_t __obj_semaphore_get_oid
	(const __object_sema_t * const o)
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

error_t __obj_create_semaphore(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		object_number_t * objectno,
		const uint32_t initial_count)
{
	__object_sema_t * no = NULL;
	error_t result = NO_ERROR;

	if ( objectno )
	{
		if ( table )
		{
			no = (__object_sema_t*)__mem_alloc(pool, sizeof(__object_sema_t));
			object_number_t objno;
			result = __obj_add_object(table, (__object_t*)no, &objno);
			if ( result == NO_ERROR )
			{
				__obj_initialise_object(&no->object, objno, SEMAPHORE_OBJ);
				no->sem_count = initial_count;
				no->sem_alloc = 0;
				no->listeners = thread_obj_queue_t_create(pool);
				no->owners = thread_obj_queue_t_create(pool);
				no->highest_priority = 0;
				no->pool = pool;
				*objectno = no->object.object_number;
			}
		}
		else
		{
			result = PARAMETERS_OUT_OF_RANGE;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

error_t __object_delete_semaphore(
		__object_sema_t * const semaphore)
{
	error_t result = NO_ERROR;
	if (semaphore)
	{
		thread_obj_queue_t_delete(semaphore->listeners);
		thread_obj_queue_t_delete(semaphore->owners);
		__mem_free(semaphore->pool, semaphore);
	}
	else
	{
		result = INVALID_OBJECT;
	}
	return result;
}

error_t __obj_get_semaphore(
		__object_thread_t * const thread,
		__object_sema_t * const semaphore)
{
	error_t result = NO_ERROR;

	if (thread && semaphore)
	{
		__thread_state_t ts;
		__obj_get_thread_state(thread, &ts);

		if (ts != thread_not_created && ts != thread_terminated)
		{
			__object_thread_t * first_owner_obj = NULL;

			__obj_lock(&semaphore->object);

			thread_obj_queue_t_front(semaphore->owners, &first_owner_obj);

			const priority_t thread_priority =
					__obj_get_thread_priority_ex(thread);

			const priority_t waiting_thread_priority =
					__obj_get_thread_priority_ex(first_owner_obj);

			/* deal with incoming priority inheritance */
			if ( (semaphore->sem_count == 0) &&
					(thread_priority
					> waiting_thread_priority) )
			{
				/* store the old priority */
				__obj_set_thread_original_priority(first_owner_obj);

				/* set the lower threads priority to the higher one
				 * and set it running */
				__obj_set_thread_priority(
						first_owner_obj,
						thread_priority);

				semaphore->highest_priority = thread_priority;

				__thread_state_t state;
				__obj_get_thread_state(first_owner_obj, &state);
				if ( state == thread_waiting )
				{
					__obj_set_thread_ready(first_owner_obj);
				}

			}
			else
			{
				/* not a priority inversion, just update the numbers to ensure
				 * that when the semaphore is released we don't accidently change
				 * a threads priority level*/
				thread_obj_queue_t_push(semaphore->owners, thread);

				semaphore->highest_priority = thread_priority;
			}

			/* if a thread has it, wait... */
			if (semaphore->sem_count == 0)
			{
				__obj_set_thread_waiting(thread, (__object_t*)semaphore);
				__obj_push_semaphore_listener(
						semaphore->listeners,
						thread);
			}
			else
			{
				semaphore->sem_count--;
				semaphore->sem_alloc++;
			}

			__obj_release(&semaphore->object);
		}
		else
		{
			result = OBJECT_IN_INVALID_STATE;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

error_t __obj_release_semaphore(
		__object_thread_t * const thread,
		__object_sema_t * const semaphore)
{
	error_t result = NO_ERROR;

	if ( semaphore )
	{
		__obj_lock(&semaphore->object);
		if ( semaphore->sem_alloc > 0)
		{
			semaphore->sem_count++;
			semaphore->sem_alloc--;

			/* the thread was a lower priority thread that had
			 * its priority temporarily elevated to avoid priority
			 * inversion so the original thread priority is restored
			 */
			const priority_t thread_orig_priority = __obj_get_thread_original_priority_ex(thread);
			if ( semaphore->highest_priority != thread_orig_priority)
			{
				thread_obj_queue_t_remove(semaphore->owners, thread);
				__obj_reset_thread_original_priority(thread);
			}
			/* need to keep the highest priority at the right level */
			else if ( semaphore->highest_priority > thread_orig_priority)
			{
				semaphore->highest_priority = thread_orig_priority;
			}

			__obj_notify_semaphore_listener(
					semaphore,
					semaphore->listeners);
		}
		else
		{
			result = SEMAPHORE_EMPTY;
		}

		__obj_release(&semaphore->object);
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

static void __obj_push_semaphore_listener(
		thread_obj_queue_t * const list,
		__object_thread_t * const thread)
{
	if (list && thread)
	{
		thread_obj_queue_t_push(list, thread);
	}
}

static void __obj_notify_semaphore_listener(
		__object_sema_t * const semaphore,
		thread_obj_queue_t * const list)
{
	if (list)
	{
		const uint32_t listener_count = thread_obj_queue_t_size(list);
		if (listener_count > 0)
		{
			__object_thread_t * next_thread = NULL;
			bool ok;

			ok = thread_obj_queue_t_front(list, &next_thread);

			if (ok && next_thread)
			{
				/* tell the most recent one to go now */
				__obj_set_thread_ready(next_thread);

				const priority_t thread_priority = __obj_get_thread_priority_ex(next_thread);
				if (thread_priority < semaphore->highest_priority)
				{
					/* update the original priority and copy across the temporary
					 * higher priority */
					__obj_set_thread_original_priority(next_thread);

					__obj_set_thread_priority(
							next_thread,
							semaphore->highest_priority);
				}
				else
				{
					/* not a priority inversion, just update the numbers to ensure
					 * that when the semaphore is released we don't accidently change
					 * a threads priority level*/
					semaphore->highest_priority = thread_priority;
				}

				thread_obj_queue_t_push(
						semaphore->owners,
						next_thread);

				thread_obj_queue_t_pop(list);
			}
		}
	}
}
