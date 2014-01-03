/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_semaphore.h"

#include "config.h"
#include "object.h"
#include "object_private.h"
#include "obj_thread.h"
#include "object_table.h"
#include "registry.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/collections/unbounded_queue.h"

UNBOUNDED_QUEUE_TYPE(__thread_obj_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_CREATE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_INITIALISE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_DELETE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_FRONT(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_PUSH(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_POP(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_REMOVE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_SPEC_SIZE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_CREATE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_INITIALISE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_DELETE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_FRONT(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_PUSH(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_POP(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_REMOVE(static,__thread_obj_queue_t, __object_thread_t*)
UNBOUNDED_QUEUE_BODY_SIZE(static,__thread_obj_queue_t, __object_thread_t*)

typedef enum
{
	unknown_sema_type = 0,
	sema_type_owner,
	sema_type_link
} sema_type_t;

typedef struct __object_sema_t
{
	__object_internal_t object;
	__mem_pool_info_t * pool;
	sema_type_t sema_type;
	union
	{
		struct
		{
			uint32_t sem_count;
			uint32_t sem_alloc;
			__thread_obj_queue_t * listeners;
			__thread_obj_queue_t * owners;
			priority_t highest_priority;
			char name[__MAX_SHARED_OBJECT_NAME_LENGTH];
		} owner;
		struct
		{
			__object_sema_t * link;
		} link;
	} data;
} __object_sema_internal_t;

static void __obj_push_semaphore_listener(
		__thread_obj_queue_t * const list,
		__object_thread_t * const object);

static void __obj_notify_semaphore_listener(
		__object_sema_t * const semaphore,
		__thread_obj_queue_t * const list);

__object_sema_t * __obj_cast_semaphore(__object_t * o)
{
	__object_sema_t * result = NULL;
	if(o)
	{
		const __object_sema_t * const tmp = (const __object_sema_t*)o;
		if (tmp->object.type == SEMAPHORE_OBJ)
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
		oid = o->object.object_number;
	}
	return oid;
}

error_t __obj_create_semaphore(
		__process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t initial_count)
{
	__object_sema_t * no = NULL;
	error_t result = NO_ERROR;

	if (objectno && process)
	{
		__object_table_t * const table = __process_get_object_table(process);
		if (table)
		{
			__mem_pool_info_t * const pool = __process_get_mem_pool(process);
			no = (__object_sema_t*)__mem_alloc(pool, sizeof(__object_sema_t));
			if (no)
			{
				object_number_t objno;
				result = __obj_add_object(table, (__object_t*)no, &objno);
				if ( result == NO_ERROR )
				{
					__obj_initialise_object(&no->object, objno, SEMAPHORE_OBJ);
					no->sema_type = sema_type_owner;
					no->data.owner.sem_count = initial_count;
					no->data.owner.sem_alloc = 0;
					no->data.owner.listeners = __thread_obj_queue_t_create(pool);
					no->data.owner.owners = __thread_obj_queue_t_create(pool);
					no->data.owner.highest_priority = 0;
					no->pool = pool;
					memset(no->data.owner.name, 0, sizeof(no->data.owner.name));
					__util_memcpy(no->data.owner.name, name, __util_strlen(name,__MAX_SHARED_OBJECT_NAME_LENGTH));
					__regsitery_add(name, process, no->object.object_number);
					*objectno = no->object.object_number;
				}
			}
			else
			{
				__mem_free(pool, no);
				result = OUT_OF_MEMORY;
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

error_t __obj_open_semaphore(
		__process_t * const process,
		object_number_t * objectno,
		const char * name)
{
	__object_sema_t * no = NULL;
	error_t result = NO_ERROR;

	if (objectno && process)
	{
		__object_table_t * const table = __process_get_object_table(process);
		if (table && process)
		{
			__process_t * other_process;
			object_number_t other_obj_no;
			if (__registry_get(name, &other_process, &other_obj_no) == NO_ERROR)
			{
				__object_t * const other_obj = __obj_get_object(
						__process_get_object_table(other_process),
						other_obj_no);
				if (other_obj)
				{
					__object_sema_t * const other_sema = __obj_cast_semaphore(other_obj);
					if (other_sema)
					{
						__mem_pool_info_t * const pool = __process_get_mem_pool(process);
						no = (__object_sema_t*)__mem_alloc(pool, sizeof(__object_sema_t));
						if (no)
						{
							object_number_t objno;
							result = __obj_add_object(table, (__object_t*)no, &objno);
							if (result == NO_ERROR)
							{
								__obj_initialise_object(&no->object, objno, SEMAPHORE_OBJ);
								no->sema_type = sema_type_link;
								no->data.link.link = other_sema;
								no->pool = pool;
								*objectno = no->object.object_number;
							}
							else
							{
								__mem_free(pool, no);
								result = OUT_OF_MEMORY;
							}
						}
						else
						{
							result = OUT_OF_MEMORY;
						}
					}
					else
					{
						result = WRONG_OBJ_TYPE;
					}
				}
				else
				{
					result = OBJECT_NOT_IN_REGISTRY;
				}
			}
			else
			{
				result = OBJECT_NOT_IN_REGISTRY;
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
		switch (semaphore->sema_type)
		{
		case unknown_sema_type:
			result = INVALID_OBJECT;
			break;
		case sema_type_owner:
			__registry_remove(semaphore->data.owner.name);
			__thread_obj_queue_t_delete(semaphore->data.owner.listeners);
			__thread_obj_queue_t_delete(semaphore->data.owner.owners);
			__mem_free(semaphore->pool, semaphore);
			break;
		case sema_type_link:
			__mem_free(semaphore->pool, semaphore);
			break;
		}
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

	if (thread && semaphore && semaphore->sema_type != unknown_sema_type)
	{
		__thread_state_t ts;
		__obj_get_thread_state(thread, &ts);

		if (ts != thread_not_created && ts != thread_terminated)
		{
			__object_thread_t * first_owner_obj = NULL;
			__object_sema_t * sema;
			if (semaphore->sema_type == sema_type_owner)
			{
				sema = semaphore;
			}
			else
			{
				sema = semaphore->data.link.link;
			}

			__thread_obj_queue_t_front(sema->data.owner.owners, &first_owner_obj);

			const priority_t thread_priority =
					__obj_get_thread_priority_ex(thread);

			const priority_t waiting_thread_priority =
					__obj_get_thread_priority_ex(first_owner_obj);

			/* deal with incoming priority inheritance */
			if ( (sema->data.owner.sem_count == 0) &&
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

				sema->data.owner.highest_priority = thread_priority;

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
				__thread_obj_queue_t_push(sema->data.owner.owners, thread);

				sema->data.owner.highest_priority = thread_priority;
			}

			/* if a thread has it, wait... */
			if (sema->data.owner.sem_count == 0)
			{
				__obj_set_thread_waiting(thread, (__object_t*)sema);
				__obj_push_semaphore_listener(
						sema->data.owner.listeners,
						thread);
			}
			else
			{
				sema->data.owner.sem_count--;
				sema->data.owner.sem_alloc++;
			}
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

	if (semaphore && semaphore->sema_type != unknown_sema_type)
	{
		__object_sema_t * sema;
		if (semaphore->sema_type == sema_type_owner)
		{
			sema = semaphore;
		}
		else
		{
			sema = semaphore->data.link.link;
		}
		if ( sema->data.owner.sem_alloc > 0)
		{
			sema->data.owner.sem_count++;
			sema->data.owner.sem_alloc--;

			__thread_obj_queue_t_remove(sema->data.owner.owners, thread);

			/* the thread was a lower priority thread that had
			 * its priority temporarily elevated to avoid priority
			 * inversion so the original thread priority is restored
			 */
			const priority_t thread_orig_priority = __obj_get_thread_original_priority_ex(thread);
			if ( sema->data.owner.highest_priority != thread_orig_priority)
			{
				__obj_reset_thread_original_priority(thread);
			}
			/* need to keep the highest priority at the right level */
			else if ( sema->data.owner.highest_priority > thread_orig_priority)
			{
				sema->data.owner.highest_priority = thread_orig_priority;
			}

			__obj_notify_semaphore_listener(
					sema,
					sema->data.owner.listeners);
		}
		else
		{
			result = SEMAPHORE_EMPTY;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}

static void __obj_push_semaphore_listener(
		__thread_obj_queue_t * const list,
		__object_thread_t * const thread)
{
	if (list && thread)
	{
		__thread_obj_queue_t_push(list, thread);
	}
}

static void __obj_notify_semaphore_listener(
		__object_sema_t * const semaphore,
		__thread_obj_queue_t * const list)
{
	if (list)
	{
		const uint32_t listener_count = __thread_obj_queue_t_size(list);
		if (listener_count > 0)
		{
			__object_thread_t * next_thread = NULL;
			bool_t ok;

			ok = __thread_obj_queue_t_front(list, &next_thread);

			if (ok && next_thread)
			{
				/* tell the most recent one to go now */
				__obj_set_thread_ready(next_thread);

				const priority_t thread_priority = __obj_get_thread_priority_ex(next_thread);
				if (thread_priority < semaphore->data.owner.highest_priority)
				{
					/* update the original priority and copy across the temporary
					 * higher priority */
					__obj_set_thread_original_priority(next_thread);

					__obj_set_thread_priority(
							next_thread,
							semaphore->data.owner.highest_priority);
				}
				else
				{
					/* not a priority inversion, just update the numbers to ensure
					 * that when the semaphore is released we don't accidently change
					 * a threads priority level*/
					semaphore->data.owner.highest_priority = thread_priority;
				}

				__thread_obj_queue_t_push(
						semaphore->data.owner.owners,
						next_thread);

				__thread_obj_queue_t_pop(list);
			}
		}
	}
}

uint32_t __obj_get_sema_count(const __object_sema_t * const sema)
{
	uint32_t count = 0;
	if (sema)
	{
		switch (sema->sema_type)
		{
		case unknown_sema_type:
			count = 0;
			break;
		case sema_type_owner:
			count = sema->data.owner.sem_count;
			break;
		case sema_type_link:
			count = sema->data.link.link->data.owner.sem_count;
			break;
		}
	}
	return count;
}

uint32_t __obj_get_sema_alloc(const __object_sema_t * const sema)
{
	uint32_t alloc = 0;
	if (sema)
	{
		switch (sema->sema_type)
		{
		case unknown_sema_type:
			alloc = 0;
			break;
		case sema_type_owner:
			alloc = sema->data.owner.sem_alloc;
			break;
		case sema_type_link:
			alloc = sema->data.link.link->data.owner.sem_alloc;
			break;
		}
	}
	return alloc;
}

priority_t __obj_get_sema_highest_priority(const __object_sema_t * const sema)
{
	priority_t pri = 0;
	if (sema)
	{
		switch (sema->sema_type)
		{
		case unknown_sema_type:
			pri = 0;
			break;
		case sema_type_owner:
			pri = sema->data.owner.highest_priority;
			break;
		case sema_type_link:
			pri = sema->data.link.link->data.owner.highest_priority;
			break;
		}
	}
	return pri;
}
