/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "object.h"
#include "obj_semaphore.h"
#include "obj_thread.h"
#include "object_table.h"
#include "kernel/scheduler/scheduler.h"

#include "kernel/utils/collections/unbounded_queue.h"

UNBOUNDED_QUEUE_TYPE(thread_obj_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(thread_obj_queue_t, __object_t*)
UNBOUNDED_QUEUE_SPEC(static,thread_obj_queue_t, __object_t*)
UNBOUNDED_QUEUE_BODY(static,thread_obj_queue_t, __object_t*)

static void __obj_push_semaphore_listener(
		thread_obj_queue_t * const list,
		__object_t * const object);

static void __obj_notify_semaphore_listener(
		__object_t * const semaphore,
		thread_obj_queue_t * const list);

error_t __obj_initialise_semaphore(
		__object_t * const object,
		const uint32_t initial_count)
{
	error_t result = NO_ERROR;

	__thread_t * current_thread = __sch_get_current_thread();
	if ( current_thread && current_thread->parent)
	{
		__obj_set_type(object, SEMAPHORE_OBJ);
		object->specifics.semaphore.sem_count = initial_count;
		object->specifics.semaphore.sem_alloc = 0;
		object->specifics.semaphore.listeners = thread_obj_queue_t_create(
				current_thread->parent->memory_pool);
		object->specifics.semaphore.owners = thread_obj_queue_t_create(
				current_thread->parent->memory_pool);
		object->specifics.semaphore.highest_priority = 0;
	}
	else
	{
		result = INVALID_CONTEXT;
	}

	return result;
}

error_t __obj_create_semaphore(
		__mem_pool_info_t * const pool,
		__object_table_t * const table,
		__object_t ** object,
		const uint32_t initial_count)
{
	__object_t * no = NULL;
	error_t result = NO_ERROR;

	if ( object )
	{
		if ( table )
		{
			if ( (result = __obj_allocate_next_free_object(pool, table, &no)) == NO_ERROR )
			{
				result = __obj_initialise_semaphore(no, initial_count);
				*object = no;
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

error_t __obj_get_semaphore(__object_t * const thread,
							__object_t * const semaphore)
{
	error_t result = NO_ERROR;

	if ( thread && semaphore )
	{
		if ( __obj_get_type(thread)==THREAD_OBJ &&
				__obj_get_type(semaphore)==SEMAPHORE_OBJ)
		{
			__thread_state_t ts;

			ts = thread->specifics.thread.thread->state;
			if (ts != thread_not_created && ts != thread_terminated)
			{
				thread_obj_queue_t * owner_list = NULL;
				__object_t * first_owner_obj = NULL;
				__thread_t * owner_thread = NULL;
				__thread_t * new_thread = NULL;

				__obj_lock(semaphore);

				owner_list = (thread_obj_queue_t*)semaphore->specifics.semaphore.owners;
				thread_obj_queue_t_front(owner_list, &first_owner_obj);
				owner_thread = first_owner_obj->specifics.thread.thread;
				new_thread = thread->specifics.thread.thread;

				/* deal with incoming priority inheritance */
				if ( (semaphore->specifics.semaphore.sem_count == 0) &&
						(new_thread->priority
						> owner_thread->priority) )
				{
					/* store the old priority */
					__obj_set_thread_original_priority(first_owner_obj);

					/* set the lower threads priority to the higher one
					 * and set it running */
					__obj_set_thread_priority(
							first_owner_obj,
							thread->specifics.thread.thread->priority);

					semaphore->specifics.semaphore.highest_priority =
							thread->specifics.thread.thread->priority;

					if ( owner_thread->state == thread_waiting )
					{
						__obj_set_thread_ready(first_owner_obj);
					}

				}
				else
				{
					/* not a priority inversion, just update the numbers to ensure
					 * that when the semaphore is released we don't accidently change
					 * a threads priority level*/
					thread_obj_queue_t_push(owner_list, thread);

					semaphore->specifics.semaphore.highest_priority =
							thread->specifics.thread.thread->priority;
				}

				/* if a thread has it, wait... */
				if (semaphore->specifics.semaphore.sem_count == 0)
				{
					__obj_set_thread_waiting(thread, semaphore);
					__obj_push_semaphore_listener(
							semaphore->specifics.semaphore.listeners,
							thread);
				}

				semaphore->specifics.semaphore.sem_count--;
				semaphore->specifics.semaphore.sem_alloc++;
				__obj_release(semaphore);
			}
			else
			{
				result = OBJECT_IN_INVALID_STATE;
			}
		}
		else
		{
			result = INVALID_OBJECT;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

error_t __obj_release_semaphore(
		__object_t * const thread,
		__object_t * const semaphore)
{
	error_t result = NO_ERROR;

	if ( semaphore )
	{
		if ( __obj_get_type(semaphore)==SEMAPHORE_OBJ)
		{
			__obj_lock(semaphore);
			if ( semaphore->specifics.semaphore.sem_alloc > 0)
			{
				semaphore->specifics.semaphore.sem_count++;
				semaphore->specifics.semaphore.sem_alloc--;

				/* the thread was a lower priority thread that had
				 * its priority temporarily elevated to avoid priority
				 * inversion so the original thread priority is restored
				 */
				if ( semaphore->specifics.semaphore.highest_priority !=
						thread->specifics.thread.original_priority)
				{
					thread_obj_queue_t * owner_list = NULL;

					owner_list = (thread_obj_queue_t*)semaphore->specifics.semaphore.owners;
					thread_obj_queue_t_remove(owner_list, thread);
					__obj_reset_thread_original_priority(thread);
				}
				/* need to keep the highest priority at the right level */
				else if ( semaphore->specifics.semaphore.highest_priority >
						  thread->specifics.thread.original_priority )
				{
					semaphore->specifics.semaphore.highest_priority =
							thread->specifics.thread.original_priority;
				}

				__obj_notify_semaphore_listener(
						semaphore,
						semaphore->specifics.semaphore.listeners);
			}
			else
			{
				result = SEMAPHORE_EMPTY;
			}
			__obj_release(semaphore);
		}
		else
		{
			result = INVALID_OBJECT;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

static void __obj_push_semaphore_listener(
		thread_obj_queue_t * const list,
		__object_t * const object)
{
	if ( list && object )
	{
		thread_obj_queue_t_push(list, object);
	}
}

static void __obj_notify_semaphore_listener(
		__object_t * const semaphore,
		thread_obj_queue_t * const list)
{
	if ( list )
	{
		const uint32_t listener_count = thread_obj_queue_t_size(list);
		if ( listener_count > 0 )
		{
			__object_t * next_thread = NULL;
			bool ok;

			ok = thread_obj_queue_t_front(list, &next_thread);

			if ( ok && next_thread)
			{
				/* tell the most recent one to go now */
				__obj_set_thread_ready(next_thread);

				if ( next_thread->specifics.thread.thread->priority <
						semaphore->specifics.semaphore.highest_priority )
				{
					/* update the original priority and copy across the temporary
					 * higher priority */
					__obj_set_thread_original_priority(next_thread);

					__obj_set_thread_priority(
							next_thread,
							semaphore->specifics.semaphore.highest_priority);
				}
				else
				{
					/* not a priority inversion, just update the numbers to ensure
					 * that when the semaphore is released we don't accidently change
					 * a threads priority level*/
					semaphore->specifics.semaphore.highest_priority =
							next_thread->specifics.thread.thread->priority;
				}

				thread_obj_queue_t_push(
						semaphore->specifics.semaphore.owners,
						next_thread);

				thread_obj_queue_t_pop(list);
			}
		}
	}
}
