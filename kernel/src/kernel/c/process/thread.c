/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "process/thread.h"
#include "process/thread_private.h"
#include "config.h"
#include "tgt.h"
#include "utils/util_strcpy.h"
#include "kernel_assert.h"
#include "tinker_api_kernel_interface.h"
#include "console/print_out.h"
#include "utils/util_memset.h"

static void thread_end(void) __attribute__((section(".api")));

static void thread_setup_stack(thread_t * const thread)
{
	const mem_t stack_size = thread->stack_size;
	const mem_t rsp = ((uint32_t)thread->stack) + (stack_size - 12);
	mem_t vsp = 0;
#if defined (KERNEL_DEBUGGING)
	bool_t is_kernel = process_is_kernel(thread->parent);
#endif
	const mem_section_t * mem = process_get_first_section(thread->parent);
	while (mem)
	{
		if (mem_sec_get_real_addr(mem) == mem_get_start_addr(process_get_user_mem_pool(thread->parent)))
		{
			vsp = mem_sec_get_virt_addr(mem)
					+ (((uint32_t)thread->stack
							- mem_get_start_addr(process_get_user_mem_pool(thread->parent)))
							+ (stack_size - 12));
			if (is_debug_enabled(PROCESS))
			{
				debug_print(PROCESS, "Process: Base %8x, Stack: %8x, Pool: %8x, Sz: %8x = [%8x]\n",
						VIRTUAL_ADDRESS_SPACE(is_kernel),
						thread->stack,
						mem_get_start_addr(process_get_user_mem_pool(thread->parent)),
						stack_size,
						vsp);
			}
			break;
		}
		mem = mem_sec_get_next(mem);
	}
	thread->r_stack_base = rsp;
	thread->v_stack_base = vsp;
}

thread_t * thread_create(
		mem_pool_info_t * const pool,
		mem_pool_info_t * const user_pool,
		const fwd_process_t * const parent,
		const priority_t priority,
		thread_entry_point * entry_point,
		const uint32_t flags,
		const uint32_t stack,
		const char * const name)
{
	thread_t * const thread = mem_alloc(pool, sizeof(thread_t));
	if (thread)
	{
		util_memset(thread, 0, sizeof(thread_t));
		thread->parent = (process_t*)parent;
		thread->thread_id = 0;
		thread->priority = priority;
		thread->entry_point = entry_point;
		thread->flags = flags;
		thread->stack = mem_alloc_aligned(user_pool, stack, MMU_PAGE_SIZE);
		if (thread->stack)
		{
			util_memset(thread->stack, 0, stack);
		}
		util_strcpy(thread->name, name, MAX_THREAD_NAME_LEN);
		debug_print(PROCESS, "Process: Created thread %s with stack size %x at %x\n", name, stack, thread->stack);
		if (thread->stack)
		{
			thread->stack_size = stack;
			thread->state = THREAD_READY;
			/*
			 * We need to ensure that the context information is configured properly
			 */
			thread_setup_stack(thread);
			tgt_initialise_context(
					thread,
					&thread->context,
					(const mem_t)thread_end);
		}
		else
		{
			thread->stack_size = 0;
			thread->state = THREAD_NOT_CREATED;
		}
	}
	return thread;
}

uint32_t thread_get_tid(const thread_t * const thread)
{
	uint32_t tid = 0;
	if (thread)
	{
		tid = thread->thread_id;
	}
	return tid;
}

const char * thread_get_name(const thread_t * const thread)
{
	const char * name = NULL;
	if (thread)
	{
		name = thread->name;
	}
	return name;
}

void thread_set_tid(
		thread_t * const thread,
		const uint32_t tid)
{
	kernel_assert("thread is null", thread != NULL);
	thread->thread_id = tid;
}

void thread_set_oid(
		thread_t * const thread,
		const object_number_t oid)
{
	kernel_assert("thread is null", thread != NULL);
	thread->object_number = oid;
}

priority_t thread_get_priority(
		const thread_t * const thread)
{
	priority_t p = 0;
	if (thread)
	{
		p = thread->priority;
	}
	return p;
}

void thread_set_priority(
		thread_t * const thread,
		const priority_t priority)
{
	kernel_assert("thread is null", thread != NULL);
	thread->priority = priority;
}

process_t * thread_get_parent(
		const thread_t * const thread)
{
	process_t * parent = NULL;
	if (thread)
	{
		parent = thread->parent;
	}
	return parent;
}

thread_state_t thread_get_state(
		const thread_t * const thread)
{
	thread_state_t state = THREAD_NOT_CREATED;
	if (thread)
	{
		state = thread->state;
	}
	return state;
}

void thread_set_state(
		thread_t * const thread,
		const thread_state_t new_state)
{
	kernel_assert("thread is null", thread != NULL);
	thread->state = new_state;
}

void thread_load_context(
		const thread_t * const thread,
		tgt_context_t * const context)
{
	kernel_assert("thread is null", thread != NULL);
	tgt_load_context(thread->context, context);
}

void thread_save_context(
		thread_t * const thread,
		const tgt_context_t * const context)
{
	if (thread && context)
	{
		debug_print(PROCESS, "Process: Saving context %8x for %s\n", context, thread->name);
		tgt_save_context(thread->context, context);
	}
}

void thread_set_context_param(
		thread_t * const thread,
		const uint8_t index,
		const uint32_t parameter)
{
	kernel_assert("thread is null", thread != NULL);
	tgt_set_context_param(thread->context, index, parameter);
}

object_number_t thread_get_object_no(
		const thread_t * const thread)
{
	object_number_t object_number = INVALID_OBJECT_ID;
	if (thread)
	{
		object_number = thread->object_number;
	}
	return object_number;
}

// FIXME should be mem_t
uint32_t thread_get_virt_stack_base(
		const thread_t * const thread)
{
	uint32_t vsb = 0;
	if (thread)
	{
		vsb = thread->v_stack_base;
	}
	return vsb;
}

const tgt_context_t * thread_get_context(
		const thread_t * const thread)
{
	return thread == NULL ? NULL : thread->context;
}

thread_entry_point * thread_get_entry_point(
		const thread_t * const thread)
{
	thread_entry_point * p = NULL;
	if (thread)
	{
		p = thread->entry_point;
	}
	return p;
}

uint32_t thread_get_flags(
		const thread_t * const thread)
{
	uint32_t flags = 0;
	if (thread)
	{
		flags = thread->flags;
	}
	return flags;
}


const object_t * thread_get_waiting_on(
		const thread_t * const thread)
{
	const object_t * waiting_on = NULL;
	if (thread)
	{
		debug_print(PROCESS, "Process: Thread %s Waiting on %x\n", thread->name, thread->waiting_on);
		waiting_on = thread->waiting_on;
	}
	return waiting_on;
}

void thread_set_waiting_on(
		thread_t * const thread,
		const object_t * const object)
{
	kernel_assert("thread is null", thread != NULL);
	debug_print(PROCESS, "Process: Thread %s Waiting on %x\n", thread->name, object);
	thread->waiting_on = object;
}

uint32_t thread_get_stack_size(const thread_t * const thread)
{
	uint32_t stack_size = 0;
	if (thread)
	{
		stack_size = thread->stack_size;
	}
	return stack_size;
}

void thread_exit(thread_t * const thread)
{
	if (thread)
	{
		mem_pool_info_t * const pool = process_get_mem_pool(thread->parent);
		mem_pool_info_t * const user_pool = process_get_user_mem_pool(thread->parent);

		// stack
		mem_free(user_pool, thread->stack);
		thread->stack = NULL;
		// contact
		tgt_destroy_context(pool, thread->context);
		thread->context = NULL;
		// thread itself
		mem_free(pool, thread);
	}
}

static void thread_end(void)
{
	TINKER_API_CALL_0(SYSCALL_EXIT_THREAD);
}
