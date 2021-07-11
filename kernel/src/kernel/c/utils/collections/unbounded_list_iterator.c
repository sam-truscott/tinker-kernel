/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "unbound_list_iterator.h"

#include "../util_memcpy.h"

typedef struct list_it_t
{
	const list_t * list;
	list_t_element_t * current;
	uint32_t pos;
} list_it_t_internal ;

/*
 * Initialise a new linked list
 */
void list_it_initialise(list_it_t * const it, const list_t * const list)
{
	 if (it)
	 {
		 util_memset(it, 0, sizeof(list_it_t));
		 it->list = list;
		 it->pos = 0;
		 it->current = NULL;
	 }
	 if (list && list->size > 0)
	 {
		 it->current = list->head;
	 }
}

/*
 * Create a new instance of an iterator for the linked list
 */
list_it_t * list_it_create(const list_t * const list)
{
	list_it_t * it = NULL;
	if (list)
	{
		it = mem_alloc(list->pool, sizeof(list_it_t));
		list_it_initialise(it, list);
	}
	return it;
}

/*
 * Delete the iterator
 */
void list_it_delete(list_it_t * const it)
{
	 if (it)
	 {
		 mem_free(it->list->pool, it);
	 }
}

/*
 * Get the current item from the list
 */
bool_t list_it_get(list_it_t * const it, void * item)
{
	bool_t ok = false;
	if (it && item)
	{
		if (!it->current)
		{
			it->current = it->list->head;
		}

		if (it->current)
		{
			util_memcpy(item, &(it->current->item), sizeof(ITEM_T));
			ok = true;
		}
	}

	return ok;
}
bool_t list_it_fwd(list_it_t * const it, uint32_t count)
{
	bool_t ok = false;
	if (it)
	{
		ok = true;
		for (uint32_t i = 0 ; i < count && ok ; i++)
		{
			it->current = it->current->next;
			it->pos++;
			ok = it->current != NULL;
		}
	}
	return ok;
}

bool_t list_it_back(list_it_t * const it, uint32_t count)
{
	bool_t ok = false;
	if (it)
	{
		ok = true;
		for (uint32_t i = 0 ; i < count && ok ; i++)
		{
			it->current = it->current->prev;
			it->pos--;
			ok = it->current != NULL;
		}
	}

	return ok;
}

/*
 * Get the next element from the list
 */
bool_t list_it_next(list_it_t * const it, void * item)
{
	 bool_t ok = false;

	 if (it && item)
	 {
		 util_memset(item, 0, sizeof(ITEM_T));
		 if (it->current)
		 {
			 if (it->current->next)
			 {
				 util_memcpy(item, &(it->current->next->item), sizeof(ITEM_T));
				 it->current = it->current->next;
				 it->pos++;
				 ok = true;
			 }
		 }
	 }

	 return ok;
}

/*
 * Get the previous element from the list
 */
bool_t list_it_prev(list_it_t * const it, void * item)
{
	 bool_t ok = false;
	 if (it && item)
	 {
		util_memset(item, 0, sizeof(ITEM_T));
		 if (it->current)
		 {
			 if (it->current->prev)
			 {
				 util_memcpy(item, &(it->current->prev->item), sizeof(ITEM_T));
				 it->current = it->current->prev;
				 it->pos--;
				 ok = true;
			 }
		 }
	 }

	 return ok;
}

void list_it_reset(list_it_t * const it)
{
	if (it)
	{
		it->current = it->list->head;
		it->pos = 0;
	}
}

uint32_t list_it_where(list_it_t const * it)
{
	return it->pos;
}
