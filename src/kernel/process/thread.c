/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel/process/thread.h"

#include "config.h"
#include "arch/tgt.h"
#include "kernel/process/process.h"
#include "kernel/objects/object.h"
#include "kernel/utils/util_strlen.h"

typedef struct __thread_t
{
	uint32_t			thread_id;
	void 				* stack;
	uint32_t			r_stack_base;
	uint32_t			v_stack_base;
	uint32_t			stack_size;
	__priority_t			priority;
	uint32_t			flags;
	__process_t 		*	parent;
	__thread_state_t	state;
	const __object_t *	waiting_on;
	thread_entry_point * 	entry_point;
	object_number_t		object_number;
	__tgt_context_t		* context;
	char 				name[__MAX_THREAD_NAME_LEN + 1];
} __thread_internal_t;

static void __thread_setup_stack(__thread_t * const thread)
{
	const uint32_t stack_size = thread->stack_size;
	const uint32_t rsp = ((uint32_t)thread->stack) + stack_size - 12;
	uint32_t vsp;

	if ( !__process_is_kernel(thread->parent) )
	{
		vsp = VIRTUAL_ADDRESS_SPACE
				+ (((uint32_t)thread->stack
						- __mem_get_start_addr(__process_get_mem_pool(thread->parent)))
						+ stack_size - 12);
	}
	else
	{
		vsp = rsp;
	}
	thread->r_stack_base = rsp;
	thread->v_stack_base = vsp;
}

__thread_t * __thread_create(
		__mem_pool_info_t * const pool,
		const __fwd_process_t * const parent,
		const __priority_t priority,
		thread_entry_point * entry_point,
		const uint32_t flags,
		const uint32_t stack,
		const char * const name)
{
	__thread_t * const thread = __mem_alloc(pool, sizeof(__thread_t));
	if (thread)
	{
		thread->parent = (__process_t*)parent;
		thread->thread_id = 0;
		thread->priority = priority;
		thread->entry_point = entry_point;
		thread->flags = flags;
		thread->stack = __mem_alloc_aligned(pool, stack, MMU_PAGE_SIZE);
		const uint32_t length = __util_strlen(name, __MAX_THREAD_NAME_LEN);
		__util_memcpy(thread->name, name, length);
		thread->name[length] = '\0';
		if (thread->stack)
		{
			thread->stack_size = stack;
			thread->state = THREADY_READY;
			/*
			 * We need to ensure that the context information
			 * is configured properly
			 */
			__thread_setup_stack(thread);
			__tgt_initialise_context(
					thread,
					&thread->context,
					__process_is_kernel(thread->parent),
					(const uint32_t)sos_exit_thread);
		}
		else
		{
			thread->stack_size = 0;
			thread->state = THREAD_NOT_CREATED;
		}
	}
	return thread;
}

uint32_t __thread_get_tid(const __thread_t * const thread)
{
	return thread->thread_id;
}

const char * __thread_get_name(const __thread_t * const thread)
{
	return thread->name;
}

void __thread_set_tid(
		__thread_t * const thread,
		const uint32_t tid)
{
	thread->thread_id = tid;
}

void __thread_set_oid(
		__thread_t * const thread,
		const object_number_t oid)
{
	thread->object_number = oid;
}

__priority_t __thread_get_priority(
		const __thread_t * const thread)
{
	return thread->priority;
}

void __thread_set_priority(
		__thread_t * const thread,
		const __priority_t priority)
{
	thread->priority = priority;
}

__process_t * __thread_get_parent(
		const __thread_t * const thread)
{
	return thread->parent;
}

__thread_state_t __thread_get_state(
		const __thread_t * const thread)
{
	return thread->state;
}

void __thread_set_state(
		__thread_t * const thread,
		const __thread_state_t new_state)
{
	thread->state = new_state;
}

void __thread_load_context(
		const __thread_t * const thread,
		__tgt_context_t * const context)
{
	__tgt_load_context(thread->context, context);
}

void __thread_save_context(
		__thread_t * const thread,
		const __tgt_context_t * const context)
{
	__tgt_save_context(thread->context, context);
}

object_number_t __thread_get_object_no(
		const __thread_t * const thread)
{
	return thread->object_number;
}

uint32_t __thread_get_virt_stack_base(
		const __thread_t * const thread)
{
	return thread->v_stack_base;
}

thread_entry_point * __thread_get_entry_point(
		const __thread_t * const thread)
{
	return thread->entry_point;
}

uint32_t __thread_get_flags(
		const __thread_t * const thread)
{
	return thread->flags;
}


const __object_t * __thread_get_waiting_on(
		const __thread_t * const thread)
{
	return thread->waiting_on;
}

void __thread_set_waiting_on(
		__thread_t * const thread,
		const __object_t * const object)
{
	thread->waiting_on = object;
}

uint32_t __thread_get_stack_size(const __thread_t * const thread)
{
	return thread->stack_size;
}

void __thread_exit(__thread_t * const thread)
{
	__mem_pool_info_t * const pool = __process_get_mem_pool(thread->parent);

	// stack
	__mem_free(pool, thread->stack);
	// contact
	__tgt_destroy_context(pool, thread->context);
	// thread itself
	__mem_free(pool, thread);
}
