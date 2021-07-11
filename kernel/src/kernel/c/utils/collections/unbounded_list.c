/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "unbounded_list.h"

#include "utils/util_memcpy.h"
#include "utils/util_memset.h"
#include "console/print_out.h"

#if defined (DEBUG_COLLECTIONS)
#define UNBOUNDED_LIST_DEBUG debug_print
#else
static inline void empty1(const char * const x, ...) {if (x){}}
#define UNBOUNDED_LIST_DEBUG empty1
#endif

/**
 * The data structure for a single element
 * in the list
 */
typedef struct lits_t_element
{
	void * item;
	struct list_t_element * next;
	struct list_t_element * prev;
} list_t_element_t;

/**
 * The data structure to use for the array
 */
typedef struct list_t
{
	uint32_t size;
	list_t_element_t * head;
	list_t_element_t * tail;
	mem_pool_info_t * pool;
} list_t_internal;

void list_initialise(list_t * const list, mem_pool_info_t * const pool)
{
	 if (list)
	 {
		 UNBOUNDED_LIST_DEBUG("unbounded list: initialising list %x, with pool %x\n", list, pool);
		 list-> pool = pool;
		 list->size = 0;
		 list->head = list->tail = NULL;
	 }
}

list_t * list_create(mem_pool_info_t * const pool)
{
	list_t * const lst = mem_alloc(pool, sizeof(list_t));
	if (lst)
	{
		list_initialise(lst, pool);
	}
	return lst;
}

void list_delete(list_t * const list)
{
	UNBOUNDED_LIST_DEBUG("unbounded list: deleting %x\n, list");
	if (list && list->pool)
	{
		UNBOUNDED_LIST_DEBUG("unbounded list: deleting %x from pool %x\n", list, list->pool);
		while(list->size)
		{
			LIST_T##_element_t * const e = list->head;
			list->head = e->next;
			mem_free(list->pool, e);
			list->size--;
		}
		mem_free(list->pool, list);
	}
}

bool_t list_add(list_t * const list, void * const item)
{
	bool_t ret = false;
	if (list)
	{
		list_t_element_t * const element = mem_alloc(
				list->pool, sizeof(list_t_element_t));
		/* initialise the new item */
		if (element)
		{
			element->item = item;
			element->next = NULL;
			element->prev = NULL;
			if (list->tail)
			{
				list_t_element_t * const prev = list->tail;
				list->tail->next = element;
				list->tail = element;
				list->tail->prev = prev;
			}
			else
			{
				list->head = list->tail = element;
			}
			list->size++;
			ret = true;
		}
	}
	return ret;
}

bool_t list_insert(list_t * const list, const uint32_t index, void * const item)
{
	bool_t ok = false;

	UNBOUNDED_LIST_DEBUG("unbounded list: insert in list %x at index %d\n", list, index);
	if (list && index <= list->size)
	{
		list_t_element_t * const new_element = mem_alloc(
				list->pool, sizeof(list_t_element_t));
		if (new_element)
		{
			/* setup our new element */
			new_element->item = item;
			new_element->next = NULL;
			new_element->prev = NULL;

			/* get the head of the list */
			list_t_element_t * element_before = list->head;
			if (index)
			{
				/* get the index we need */
				for (uint32_t p = 0 ; p < index - 1; p++ )
				{
					if (element_before->next)
					{
						element_before = element_before->next;
					}
					else
					{
						element_before = NULL;
						break;
					}
				}
			}
			/* setup our pointers correctly */
			if (element_before)
			{
				/* before something */
				if (element_before->next)
				{
					new_element->next = element_before->next;
					if (element_before->next)
					{
						element_before->next->prev = new_element;
					}
				}

				/* the last thing */
				if (element_before == list->tail)
				{
					list->tail = new_element;
				}

				new_element->prev = element_before;
				element_before->next = new_element;
				ok = true;
				list->size++;
			}
			else if (!list->size)
			{
				list->head = list->tail = new_element;
				ok = true;
				list->size++;
			}
			else
			{
				mem_free(list->pool, new_element);
			}
		}
	}
	return ok;
}

bool_t list_remove(list_t * const list, const uint32_t index)
{
	bool_t ret = false;

	UNBOUNDED_LIST_DEBUG("unbounded list: removing index %d from list %x\n", index, list);
	if (list)
	{
		if (list->size > index)
		{
			list_t_element_t * e = list->head;
			list_t_element_t * p = e;
			/*
			 * find the right element, keeping track
			 * of the previous one
			 */
			for (uint32_t c = 0 ; c < index ; c++ )
			{
				if (e && e->next)
				{
					p = e;
					e = e->next;
				}
				else
				{
					e = NULL; p = NULL; break;
				}
			}
			/*
			 * if the previous node isnt this node we
			 * need to update the previous node
			 */
			if (p && p != e)
			{
				/*
				 * if the next node is empty we need to
				 * update the previous node to point to
				 * nothing. other it needs to point to the
				 * next node to ensure it continues the list
				 */
				if ( e->next != NULL )
				{
					p->next = e->next;
				}
				else
				{
					p->next = NULL;
				}
				e->prev = p;
			}

			/*
			 * Update the tail of the list
			 */
			if (list->size == 1)
			{
				list->tail = list->head = NULL;
			}
			else
			{
				if (e == list->tail)
				{
					list->tail = p;
				}
				if (e == list->head)
				{
					list->head = e->next;
				}
			}

			mem_free(list->pool, e);
			list->size--;
			ret = true;
		}
	}
	UNBOUNDED_LIST_DEBUG("unbounded list: size is %d after remove %x\n", list->size, list);

	return ret;
}

/**
 * Remove an item from the list
 */
bool_t list_remove_item(list_t * const list, void * const item)
{
	bool_t ret = false;

	if (list && list->size)
	{
		list_t_element_t * e = list->head;
		for (uint32_t p = 0 ; p < list->size ; p++ )
		{
			if (e->item == item)
			{
				UNBOUNDED_LIST_DEBUG("unbounded list: removing item %d from list %x\n", p, list);
				ret = LIST_T##_remove(list, p);
				break;
			}
			e = e->next;
		}
	}
	return ret;
}


bool_t list_contains(list * const list, void * const item)
{
	bool_t ret = false;

	if (list && list->size)
	{
		LIST_T##_element_t * e = list->head;
		for (uint32_t p = 0 ; p < list->size ; p++ )
		{
			if (e->item == item)
			{
				ret = true;
				break;
			}
			e = e->next;
		}
	}

	return ret;
}


bool_t list_remove_tail(list * const list)
{
	bool_t ret = false;

	if (list)
	{
		if (list->tail)
		{
			LIST_T##_element_t * e = list->tail;
			if (list->head == list->tail)
			{
				list->head = list->tail = NULL;
			}
			else
			{
				LIST_T##_element_t * pe = list->tail->prev;
				pe->next = NULL;
				list->tail = pe;
			}
			mem_free(list->pool, e);
			list->size--;
			ret = true;
		}
	}

	return ret;
}

bool_t list_head_to_tail(list_t * const list)
{
	bool_t ret = false;

	if (list)
	{
		if (list->size <= 1) /* 0 or 1 implicitly works */
		{
			ret = true;
		}
		else if (list->size > 1)
		{
			list_t_element_t * const oldHeadNext = list->head->next;
			list_t_element_t * const oldTailPrev = list->tail->prev;
			list_t_element_t * const oldTail = list->tail;
			list_t_element_t * const oldHead = list->head;
			list->head = oldTail;
			list->head->prev = NULL;
			list->head->next = oldHeadNext;
			list->tail = oldHead;
			list->tail->prev = oldTailPrev;
			list->tail->next = NULL;
			oldTailPrev->next = list->tail;
			oldHeadNext->prev = list->head;
			ret = true;
		}
	}

	return ret;
}

/**
 * Get an entry from the list
 */
bool_t list_get(const LIST_T * const list, const uint32_t index, void * const item_ptr)
{
	bool_t ret = false;

	if (list && list->size > index)
	{
		list_t_element_t * e = list->head;
		for (uint32_t p = 0 ; p < index ; p++ )
		{
			if (e->next)
			{
				e = e->next;
			}
			else
			{
				e = NULL;
				break;
			}
		}
		if (e && item_ptr)
		{
			util_memcpy(item_ptr, &e->item, sizeof(ITEM_T));
			ret = true;
		}
	}
	return ret;
}

bool_t list_get_tail(const list * const list, void * const item_ptr)
{
	bool_t ret = false;

	if (list && list->tail)
	{
		util_memcpy(item_ptr, &list->tail->item, sizeof(ITEM_T));
		ret = true;
	}

	return ret;
}

/**
 * Return the next element, if one, or return NULL if there isn't
 */
bool_t list_next(const list_t * const list, const ITEM_T current, ITEM_T * const next_ptr)
{
	bool_t ret = false;

	LIST_T##_element_t * e = list->head;
	for (uint32_t p = 0 ; p < list->size ; p++ )
	{
		if (e->item == current)
		{
			break;
		}
		else
		{
			e = e->next;
		}
	}
	if (e->next)
	{
		e = e->next;
		if (next_ptr)
		{
			util_memcpy(next_ptr, &e->item, sizeof(ITEM_T));
			ret = true;
		}
	}
	return ret;
}

/**
 * Get the size of the list
 */
uint32_t list_size(const list_t * const list)
{
	return list->size;
}
