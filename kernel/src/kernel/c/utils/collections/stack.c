/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "stack.h"

#include "utils/collections/unbounded_list.h"
#include "utils/util_memset.h"
#include "memory/memory_manager.h"

typedef struct stack_t
{
	list_t * list;
} stack_t_internal;

list_t * stack_list(stack_t * const stack)
{
	return stack->list;
}

void stack_initialise(stack_t * const stack, mem_pool_info_t * const pool)
{
	if (stack)
	{
		stack->list = list_create(pool);
	}
}

stack_t * stack_create(mem_pool_info_t * const pool)
{
	stack_t * stack = NULL;
	if (pool)
	{
		stack = (stack_t*)mem_alloc(pool, sizeof(stack_t));
		stack_initialise(stack, pool);
	}
	return stack;
}

void stack_delete(stack_t * const stack)
{
	if (stack)
	{
		mem_pool_info_t * pool = NULL;
		if (stack->list)
		{
			pool = stack->list->pool;
			list_delete(stack->list);
		}
		if (pool != NULL)
		{
			mem_free(pool, stack);
		}
	}
}

bool stack_push(stack_t * const stack, void * const item)
{
	bool_t ok = false;
	if (stack)
	{
		ok = list_add(stack->list, item);
	}
	return ok;
}

bool_t stack_insert(stack_t * const stack, const uint32_t index, void * const item)
{
	bool_t ok = false;
	if (stack)
	{
		ok = list_insert(stack->list, index, item);
	}
	return ok;
}

bool_t stack_pop(stack_t * const stack, void * const item)
{
	bool_t ok = false;

	if (stack && item)
	{
		if ((ok = list_get_tail(stack->list, item)))
		{
			list_remove_tail(stack->list);
		}
	}

	return ok;
}

bool_t stack_front(const stack_t * const stack, void * const item)
{
	bool_t ok = false;

	if (stack && item)
	{
		ok = list_get_tail(stack->list, item);
	}
	return ok;
}

bool_t stack_get(const stack_t * const stack, const uint32_t index, void * const item)
{
	bool_t ok = false;

	if (stack && item)
	{
		ok = list_get(stack->list, index, item);
	}

	return ok;
}

uint32_t stack_size(const stack_t * const stack)
{
	uint32_t size = 0;

	if (stack && stack->list)
	{
		size = list_size(stack->list);
	}

	return size;
}
