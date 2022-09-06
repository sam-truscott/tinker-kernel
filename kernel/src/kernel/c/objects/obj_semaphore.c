/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "objects/obj_semaphore.h"

#include "config.h"
#include "objects/object.h"
#include "objects/object_private.h"
#include "objects/obj_thread.h"
#include "objects/object_table.h"
#include "scheduler/scheduler.h"
#include "utils/util_strlen.h"
#include "utils/util_memset.h"
#include "utils/util_memcpy.h"
#include "utils/collections/unbounded_queue.h"

typedef enum
{
	unknown_sema_type = 0,
	sema_type_owner,
	sema_type_link
} sema_type_t;

typedef struct object_sema_t
{
	object_internal_t object;
	mem_pool_info_t * pool;
	sema_type_t sema_type;
	registry_t * reg;
	union
	{
		struct
		{
			uint32_t sem_count;
			uint32_t sem_alloc;
			queue_t * listeners;
			queue_t * owners;
			priority_t highest_priority;
			char name[MAX_SHARED_OBJECT_NAME_LENGTH];
		} owner;
		struct
		{
			object_sema_t * link;
		} link;
	} data;
} object_sema_internal_t;

static void obj_push_semaphore_listener(
		queue_t * const list,
		object_thread_t * const object);

static void obj_notify_semaphore_listener(
		object_sema_t * const semaphore,
		queue_t * const list);

object_sema_t * obj_cast_semaphore(object_t * o)
{
	object_sema_t * result = NULL;
	if(o)
	{
		const object_sema_t * const tmp = (const object_sema_t*)o;
		if (tmp->object.type == SEMAPHORE_OBJ)
		{
			result = (object_sema_t*)tmp;
		}
	}
	return result;
}

object_number_t obj_semaphore_get_oid
	(const object_sema_t * const o)
{
	object_number_t oid = INVALID_OBJECT_ID;
	if (o)
	{
		oid = o->object.object_number;
	}
	return oid;
}

return_t obj_create_semaphore(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name,
		const uint32_t initial_count)
{
	object_sema_t * no = NULL;
	return_t result = NO_ERROR;

	if (objectno && process)
	{
		object_table_t * const table = process_get_object_table(process);
		if (table)
		{
			mem_pool_info_t * const pool = process_get_mem_pool(process);
			no = (object_sema_t*)mem_alloc(pool, sizeof(object_sema_t));
			if (no)
			{
				object_number_t objno;
				result = obj_add_object(table, (object_t*)no, &objno);
				if ( result == NO_ERROR )
				{
					obj_initialise_object(&no->object, objno, SEMAPHORE_OBJ);
					no->sema_type = sema_type_owner;
					no->data.owner.sem_count = initial_count;
					no->data.owner.sem_alloc = 0;
					no->data.owner.listeners = queue_create(pool);
					no->data.owner.owners = queue_create(pool);
					no->data.owner.highest_priority = 0;
					no->pool = pool;
					no->reg = reg;
					util_memset(no->data.owner.name, 0, sizeof(no->data.owner.name));
					util_memcpy(no->data.owner.name, name, util_strlen(name,MAX_SHARED_OBJECT_NAME_LENGTH));
					regsitery_add(reg, name, process, no->object.object_number);
					*objectno = no->object.object_number;
				}
			}
			else
			{
				mem_free(pool, no);
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

return_t obj_open_semaphore(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * name)
{
	object_sema_t * no = NULL;
	return_t result = NO_ERROR;

	if (objectno && process)
	{
		object_table_t * const table = process_get_object_table(process);
		if (table && process)
		{
			process_t * other_process;
			object_number_t other_obj_no;
			if (registry_get(reg, name, &other_process, &other_obj_no) == NO_ERROR)
			{
				object_t * const other_obj = obj_get_object(
						process_get_object_table(other_process),
						other_obj_no);
				if (other_obj)
				{
					object_sema_t * const other_sema = obj_cast_semaphore(other_obj);
					if (other_sema)
					{
						mem_pool_info_t * const pool = process_get_mem_pool(process);
						no = (object_sema_t*)mem_alloc(pool, sizeof(object_sema_t));
						if (no)
						{
							object_number_t objno;
							result = obj_add_object(table, (object_t*)no, &objno);
							if (result == NO_ERROR)
							{
								obj_initialise_object(&no->object, objno, SEMAPHORE_OBJ);
								no->sema_type = sema_type_link;
								no->data.link.link = other_sema;
								no->pool = pool;
								no->reg = reg;
								*objectno = no->object.object_number;
							}
							else
							{
								mem_free(pool, no);
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

return_t object_delete_semaphore(
		object_sema_t * const semaphore)
{
	return_t result = NO_ERROR;
	if (semaphore)
	{
		switch (semaphore->sema_type)
		{
		case sema_type_owner:
			registry_remove(semaphore->reg, semaphore->data.owner.name);
			queue_delete(semaphore->data.owner.listeners);
			queue_delete(semaphore->data.owner.owners);
			mem_free(semaphore->pool, semaphore);
			break;
		case sema_type_link:
			mem_free(semaphore->pool, semaphore);
			break;
		case unknown_sema_type:
		default:
			result = INVALID_OBJECT;
			break;
		}
	}
	else
	{
		result = INVALID_OBJECT;
	}
	return result;
}

return_t obj_get_semaphore(
		object_thread_t * const thread,
		object_sema_t * const semaphore)
{
	return_t result = NO_ERROR;

	if (thread && semaphore && semaphore->sema_type != unknown_sema_type)
	{
		thread_state_t ts;
		obj_get_thread_state(thread, &ts);

		if (ts != THREAD_NOT_CREATED && ts != THREAD_TERMINATED)
		{
			object_thread_t * first_owner_obj = NULL;
			object_sema_t * sema;
			if (semaphore->sema_type == sema_type_owner)
			{
				sema = semaphore;
			}
			else
			{
				sema = semaphore->data.link.link;
			}

			queue_front(sema->data.owner.owners, &first_owner_obj);

			const priority_t thread_priority =
					obj_get_thread_priority_ex(thread);

			const priority_t waiting_thread_priority =
					obj_get_thread_priority_ex(first_owner_obj);

			/* deal with incoming priority inheritance */
			if ( (sema->data.owner.sem_count == 0) &&
					(thread_priority
					> waiting_thread_priority) )
			{
				/* store the old priority */
				obj_set_thread_original_priority(first_owner_obj);

				/* set the lower threads priority to the higher one
				 * and set it running */
				obj_set_thread_priority(
						first_owner_obj,
						thread_priority);

				sema->data.owner.highest_priority = thread_priority;

				thread_state_t state;
				obj_get_thread_state(first_owner_obj, &state);
				if ( state == THREAD_WAITING )
				{
					obj_set_thread_ready(first_owner_obj);
				}

			}
			else
			{
				/* not a priority inversion, just update the numbers to ensure
				 * that when the semaphore is released we don't accidently change
				 * a threads priority level*/
				queue_push(sema->data.owner.owners, thread);

				sema->data.owner.highest_priority = thread_priority;
			}

			/* if a thread has it, wait... */
			if (sema->data.owner.sem_count == 0)
			{
				obj_set_thread_waiting(thread, (object_t*)sema);
				obj_push_semaphore_listener(
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

return_t obj_release_semaphore(
		object_thread_t * const thread,
		object_sema_t * const semaphore)
{
	return_t result = NO_ERROR;

	if (semaphore && semaphore->sema_type != unknown_sema_type)
	{
		object_sema_t * sema;
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

			queue_remove(sema->data.owner.owners, thread);

			/* the thread was a lower priority thread that had
			 * its priority temporarily elevated to avoid priority
			 * inversion so the original thread priority is restored
			 */
			const priority_t thread_orig_priority = obj_get_thread_original_priority_ex(thread);
			if ( sema->data.owner.highest_priority != thread_orig_priority)
			{
				obj_reset_thread_original_priority(thread);
			}
			/* need to keep the highest priority at the right level */
			else if ( sema->data.owner.highest_priority > thread_orig_priority)
			{
				sema->data.owner.highest_priority = thread_orig_priority;
			}

			obj_notify_semaphore_listener(
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

static void obj_push_semaphore_listener(
		queue_t * const list,
		object_thread_t * const thread)
{
	if (list && thread)
	{
		queue_push(list, thread);
	}
}

static void obj_notify_semaphore_listener(
		object_sema_t * const semaphore,
		queue_t * const list)
{
	if (list)
	{
		const uint32_t listener_count = queue_size(list);
		if (listener_count > 0)
		{
			object_thread_t * next_thread = NULL;
			bool_t ok;

			ok = queue_front(list, &next_thread);

			if (ok && next_thread)
			{
				/* tell the most recent one to go now */
				obj_set_thread_ready(next_thread);

				const priority_t thread_priority = obj_get_thread_priority_ex(next_thread);
				if (thread_priority < semaphore->data.owner.highest_priority)
				{
					/* update the original priority and copy across the temporary
					 * higher priority */
					obj_set_thread_original_priority(next_thread);

					obj_set_thread_priority(
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

				queue_push(
						semaphore->data.owner.owners,
						next_thread);

				queue_pop(list);
			}
		}
	}
}

uint32_t obj_get_sema_count(const object_sema_t * const sema)
{
	uint32_t count = 0;
	if (sema)
	{
		switch (sema->sema_type)
		{
		case sema_type_owner:
			count = sema->data.owner.sem_count;
			break;
		case sema_type_link:
			count = sema->data.link.link->data.owner.sem_count;
			break;
		case unknown_sema_type:
		default:
			count = 0;
			break;
		}
	}
	return count;
}

uint32_t obj_get_sema_alloc(const object_sema_t * const sema)
{
	uint32_t alloc = 0;
	if (sema)
	{
		switch (sema->sema_type)
		{
		case sema_type_owner:
			alloc = sema->data.owner.sem_alloc;
			break;
		case sema_type_link:
			alloc = sema->data.link.link->data.owner.sem_alloc;
			break;
		case unknown_sema_type:
		default:
			alloc = 0;
			break;
		}
	}
	return alloc;
}

priority_t obj_get_sema_highest_priority(const object_sema_t * const sema)
{
	priority_t pri = 0;
	if (sema)
	{
		switch (sema->sema_type)
		{
		case sema_type_owner:
			pri = sema->data.owner.highest_priority;
			break;
		case sema_type_link:
			pri = sema->data.link.link->data.owner.highest_priority;
			break;
		case unknown_sema_type:
		default:
			pri = 0;
			break;
		}
	}
	return pri;
}
