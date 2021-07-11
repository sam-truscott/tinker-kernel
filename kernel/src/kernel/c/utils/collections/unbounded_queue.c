/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "utils/collections/unbounded_queue.h"


#include "utils/util_memcpy.h"
#include "unbounded_list.h"

typedef struct queue_t
{
	list_t * list;
} queue_t_internal;

void queue_initialise(queue_t * const queue, mem_pool_info_t * const pool)
{
	if (queue)
	{
		queue->list = list_create(pool);
	}
}

queue_t * queue_create(mem_pool_info_t * const pool)
{
	queue_t * queue = NULL;

	if (pool)
	{
		queue = (queue_t*)mem_alloc(pool, sizeof(queue_t));
		if (queue)
		{
			queue_initialise(queue, pool);
		}
	}
	return queue;
}

void queue_delete(queue_t * const queue)
{
	if (queue)
	{
		if (queue->list)
		{
			mem_pool_info_t * const pool = queue->list->pool;
			list_delete(queue->list);
			mem_free(pool, queue);
		}
	}
}

bool_t queue_push(queue_t * const queue, void * const item)
{
	bool_t ok = false;
	if (queue && queue->list)
	{
		ok = list_add(queue->list, item);
	}
	return ok;
}

bool_t queue_pop(queue_t * const queue)
{
	bool_t ok = false;
	if (queue && queue->list)
	{
		ok = list_remove(queue->list, 0);
	}
	return ok;
}

bool_t queue_front(const queue_t * const queue, void * const item_ptr)
{
	bool_t ok = false;
	if (queue && queue->list)
	{
		ok = list_get(queue->list, 0, item_ptr);
	}
	return ok;
}

bool_t queue_reorder_first(queue_t * const queue)
{
	bool_t ok = false;
	if (queue && queue->list)
	{
		ok = list_head_to_tail(queue->list);
	}
	return ok;
}

uint32_t queue_size(const queue_t * const queue)
{
	uint32_t size = 0;
	if (queue && queue->list)
	{
		size = list_size(queue->list);
	}
	return size;
}

bool_t queue_remove(queue_t * const queue, void * const item)
{
	bool_t ret = false;

	if (queue && queue->list)
	{
		ret = list_remove_item(queue->list, item);
	}

	return ret;
}

bool_t queue_contains(queue_t * const queue, void * item)
{
	bool_t ret = false;

	if (queue && queue->list)
	{
		ret = list_contains(queue->list, item);
	}

	return ret;
}
