/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_LIST_H_
#define UNBOUNDED_LIST_H_

#include "tinker_api_types.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/utils/util_memset.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/console/print_out.h"

#if defined (DEBUG_COLLECTIONS)
#define UNBOUNDED_LIST_DEBUG debug_print
#else
static inline void empty1(const char * const x, ...) {if (x){}}
#define UNBOUNDED_LIST_DEBUG empty1
#endif

#define UNBOUNDED_LIST_TYPE(LIST_T) \
	\
	typedef struct LIST_T LIST_T; \

#define UNBOUNDED_LIST_INTERNAL_TYPE(LIST_T, ITEM_T) \
	\
	/**
	 * The data structure for a single element
	 * in the list
	 */ \
	typedef struct LIST_T##_element \
	{ \
		ITEM_T item; \
		struct LIST_T##_element * next; \
		struct LIST_T##_element * prev; \
	} LIST_T##_element_t; \
	\
	/**
	 * The data structure to use for the array
	 */ \
	typedef struct LIST_T \
	{ \
		uint32_t size; \
		LIST_T##_element_t * head; \
		LIST_T##_element_t * tail; \
		mem_pool_info_t * pool; \
	} LIST_T##_internal; \

/**
 * This abstract data type is used to model a generic
 * single (forward) linked list.
 *
 * The list must first be created or, if static, initialised using
 * _create or _initialise.
 *
 * The user can then use _add, _get and _remove to modify the
 * values in the list.
 *
 * The size of the list can be queried with _size.
 *
 * Once the user has finished, and if the list was created dynamically
 * with the _create function, the _delete procedure should be used
 * to free the memory on the heap.
 */
#define UNBOUNDED_LIST_SPEC_INITIALISE(PREFIX, LIST_T, ITEM_T) \
	PREFIX void LIST_T##_initialise(LIST_T * const list, mem_pool_info_t * const pool);
#define UNBOUNDED_LIST_SPEC_CREATE(PREFIX, LIST_T, ITEM_T) \
	PREFIX LIST_T * LIST_T##_create(mem_pool_info_t * const pool);
#define UNBOUNDED_LIST_SPEC_DELETE(PREFIX, LIST_T, ITEM_T) \
	PREFIX void LIST_T##_delete(LIST_T * const list);
#define UNBOUNDED_LIST_SPEC_ADD(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_add(LIST_T * const list, ITEM_T item);
#define UNBOUNDED_LIST_SPEC_INSERT(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_insert(LIST_T * const list, const uint32_t index, ITEM_T item);
#define UNBOUNDED_LIST_SPEC_REMOVE(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_remove(LIST_T * const list, const uint32_t index);
#define UNBOUNDED_LIST_SPEC_REMOVE_ITEM(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_remove_item(LIST_T * const list, ITEM_T item);
#define UNBOUNDED_LIST_SPEC_REMOVE_TAIL(PREFIX, LIST_T) \
	PREFIX bool_t LIST_T##_remove_tail(LIST_T * const list);
#define UNBOUNDED_LIST_SPEC_HEAD_TO_TAIL(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_head_to_tail(LIST_T * const list);
#define UNBOUNDED_LIST_SPEC_GET(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_get(const LIST_T * const list, const uint32_t index, ITEM_T * const item_ptr);
#define UNBOUNDED_LIST_SPEC_GET_TAIL(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_get_tail(const LIST_T * const list, ITEM_T * const item_ptr);
#define UNBOUNDED_LIST_SPEC_NEXT(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_next(const LIST_T * const list, const ITEM_T current, ITEM_T * const next_ptr);
#define UNBOUNDED_LIST_SPEC_SIZE(PREFIX, LIST_T) \
	PREFIX inline uint32_t LIST_T##_size(const LIST_T * const list);
#define UNBOUNDED_LIST_SPEC_CONTAINS(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_contains(LIST_T * const list, ITEM_T item);

#define UNBOUNDED_LIST_BODY_INITIALISE(PREFIX, LIST_T, ITEM_T) \
	\
	/**
	 * Initialise the data structure
	 */ \
	PREFIX void LIST_T##_initialise(LIST_T * const list, mem_pool_info_t * const pool) \
	{ \
		 if (list) \
		 { \
			 UNBOUNDED_LIST_DEBUG("unbounded list: initialising list %x, with pool %x\n", list, pool); \
			 list-> pool = pool; \
			 list->size = 0; \
			 list->head = list->tail = NULL; \
		 } \
	}
#define UNBOUNDED_LIST_BODY_CREATE(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Constructor, returns a list on the heap or
	 * NULL if it failed.
	 */ \
	PREFIX LIST_T * LIST_T##_create(mem_pool_info_t * const pool) \
	{ \
		LIST_T * const lst = mem_alloc(pool, sizeof(LIST_T)); \
		if (lst) \
		{ \
			LIST_T##_initialise(lst, pool); \
		} \
		return lst; \
	}
#define UNBOUNDED_LIST_BODY_DELETE(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Delete the list and every element
	 */ \
	PREFIX void LIST_T##_delete(LIST_T * const list) \
	{ \
		UNBOUNDED_LIST_DEBUG("unbounded list: deleting %x\n, list"); \
		if (list && list->pool) \
		{ \
			UNBOUNDED_LIST_DEBUG("unbounded list: deleting %x from pool %x\n", list, list->pool); \
			while(list->size) \
			{ \
				LIST_T##_element_t * const e = list->head; \
				list->head = e->next; \
				mem_free(list->pool, e); \
				list->size--; \
			} \
			mem_free(list->pool, list); \
		} \
	}
#define UNBOUNDED_LIST_BODY_ADD(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Add an entry to the list
	 */ \
	PREFIX bool_t LIST_T##_add(LIST_T * const list, ITEM_T item) \
	{ \
		bool_t ret = false; \
		\
		if (list) \
		{ \
			LIST_T##_element_t * const element = mem_alloc( \
					list->pool, sizeof(LIST_T##_element_t)); \
			/* initialise the new item */ \
			if (element) \
			{ \
				element->item = item; \
				element->next = NULL; \
				element->prev = NULL; \
				if (list->tail) \
				{ \
					LIST_T##_element_t * const prev = list->tail; \
					list->tail->next = element; \
					list->tail = element; \
					list->tail->prev = prev; \
				} \
				else \
				{ \
					list->head = list->tail = element; \
				} \
				list->size++; \
				ret = true; \
			} \
		} \
		\
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_INSERT(PREFIX, LIST_T, ITEM_T) \
	\
	PREFIX bool_t LIST_T##_insert(LIST_T * const list, const uint32_t index, ITEM_T item) \
	{ \
		bool_t ok = false; \
		\
		UNBOUNDED_LIST_DEBUG("unbounded list: insert in list %x at index %d\n", list, index); \
		if (list && index <= list->size) \
		{ \
			LIST_T##_element_t * const new_element = mem_alloc( \
					list->pool, sizeof(LIST_T##_element_t)); \
			if (new_element) \
			{ \
				/* setup our new element */ \
				new_element->item = item; \
				new_element->next = NULL; \
				new_element->prev = NULL; \
				\
				/* get the head of the list */ \
				LIST_T##_element_t * element_before = list->head; \
				if (index) \
				{ \
					/* get the index we need */ \
					for (uint32_t p = 0 ; p < index - 1; p++ ) \
					{ \
						if (element_before->next) \
						{ \
							element_before = element_before->next; \
						} \
						else \
						{ \
							element_before = NULL; \
							break; \
						} \
					} \
				} \
				/* setup our pointers correctly */ \
				if (element_before) \
				{ \
					/* before something */ \
					if (element_before->next) \
					{ \
						new_element->next = element_before->next; \
						if (element_before->next) \
						{ \
							element_before->next->prev = new_element; \
						} \
					} \
					\
					/* the last thing */ \
					if (element_before == list->tail) \
					{ \
						list->tail = new_element; \
					} \
					\
					new_element->prev = element_before; \
					element_before->next = new_element; \
					ok = true; \
					list->size++; \
				} \
				else if (!list->size) \
				{ \
					list->head = list->tail = new_element; \
					ok = true; \
					list->size++; \
				} \
				else \
				{ \
					mem_free(list->pool, new_element); \
				} \
			} \
		} \
		return ok; \
	}
#define UNBOUNDED_LIST_BODY_REMOVE(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Delete an entry from the list
	 */ \
	PREFIX bool_t LIST_T##_remove(LIST_T * const list, const uint32_t index) \
	{ \
		bool_t ret = false; \
		\
		UNBOUNDED_LIST_DEBUG("unbounded list: removing index %d from list %x\n", index, list); \
		if (list) \
		{ \
			if (list->size > index) \
			{ \
				LIST_T##_element_t * e = list->head; \
				LIST_T##_element_t * p = e; \
				/*
				 * find the right element, keeping track
				 * of the previous one
				 */ \
				for (uint32_t c = 0 ; c < index ; c++ ) \
				{ \
					if (e && e->next) \
					{ \
						p = e; \
						e = e->next; \
					} \
					else \
					{ \
						e = NULL; p = NULL; break; \
					} \
				} \
				/*
				 * if the previous node isnt this node we
				 * need to update the previous node
				 */ \
				if (p && p != e) \
				{ \
					/*
					 * if the next node is empty we need to
					 * update the previous node to point to
					 * nothing. other it needs to point to the
					 * next node to ensure it continues the list
					 */ \
					if ( e->next != NULL )\
					{ \
						p->next = e->next; \
					} \
					else \
					{ \
						p->next = NULL; \
					} \
					e->prev = p; \
				} \
				\
				/*
				 * Update the tail of the list
				 */ \
				if (list->size == 1) \
				{ \
					list->tail = list->head = NULL; \
				} \
				else \
				{ \
					if (e == list->tail) \
					{ \
						list->tail = p; \
					} \
					if (e == list->head) \
					{ \
						list->head = e->next; \
					} \
				} \
				\
				mem_free(list->pool, e); \
				list->size--; \
				ret = true; \
			} \
		} \
		UNBOUNDED_LIST_DEBUG("unbounded list: size is %d after remove %x\n", list->size, list); \
		\
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_REMOVE_ITEM(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Remove an item from the list
	 */ \
	PREFIX bool_t LIST_T##_remove_item(LIST_T * const list, ITEM_T item) \
	{ \
		bool_t ret = false; \
		\
		if (list && list->size) \
		{ \
			LIST_T##_element_t * e = list->head; \
			for (uint32_t p = 0 ; p < list->size ; p++ ) \
			{ \
				if (e->item == item) \
				{ \
					UNBOUNDED_LIST_DEBUG("unbounded list: removing item %d from list %x\n", p, list); \
					ret = LIST_T##_remove(list, p); \
					break; \
				} \
				e = e->next; \
			} \
		} \
		return ret; \
	}

#define UNBOUNDED_LIST_BODY_CONTAINS(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_contains(LIST_T * const list, ITEM_T item) \
	{ \
		bool_t ret = false; \
		\
		if (list && list->size) \
		{ \
			LIST_T##_element_t * e = list->head; \
			for (uint32_t p = 0 ; p < list->size ; p++ ) \
			{ \
				if (e->item == item) \
				{ \
					ret = true; \
					break; \
				} \
				e = e->next; \
			} \
		} \
		\
		return ret; \
	}

#define UNBOUNDED_LIST_BODY_REMOVE_TAIL(PREFIX, LIST_T) \
	PREFIX bool_t LIST_T##_remove_tail(LIST_T * const list) \
	{ \
		bool_t ret = false; \
		\
		if (list) \
		{ \
			if (list->tail) \
			{ \
				LIST_T##_element_t * e = list->tail; \
				if (list->head == list->tail) \
				{ \
					list->head = list->tail = NULL; \
				} \
				else \
				{ \
					LIST_T##_element_t * pe = list->tail->prev; \
					pe->next = NULL; \
					list->tail = pe; \
				} \
				mem_free(list->pool, e); \
				list->size--; \
				ret = true; \
			} \
		} \
		\
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_HEAD_TO_TAIL(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_head_to_tail(LIST_T * const list) \
	{ \
		bool_t ret = false; \
		\
		if (list) \
		{ \
			if (list->size <= 1) /* 0 or 1 implicitly works */ \
			{ \
				ret = true; \
			} \
			else if (list->size > 1) \
			{ \
				LIST_T##_element_t * const oldHeadNext = list->head->next; \
				LIST_T##_element_t * const oldTailPrev = list->tail->prev; \
				LIST_T##_element_t * const oldTail = list->tail; \
				LIST_T##_element_t * const oldHead = list->head; \
				list->head = oldTail; \
				list->head->prev = NULL; \
				list->head->next = oldHeadNext; \
				list->tail = oldHead; \
				list->tail->prev = oldTailPrev; \
				list->tail->next = NULL; \
				oldTailPrev->next = list->tail; \
				oldHeadNext->prev = list->head; \
				ret = true; \
			} \
		} \
		\
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_GET(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Get an entry from the list
	 */ \
	PREFIX bool_t LIST_T##_get(const LIST_T * const list, const uint32_t index, ITEM_T * const item_ptr) \
	{ \
		bool_t ret = false; \
		\
		if (list && list->size > index) \
		{ \
			LIST_T##_element_t * e = list->head; \
			for (uint32_t p = 0 ; p < index ; p++ ) \
			{ \
				if (e->next) \
				{ \
					e = e->next; \
				} \
				else \
				{ \
					e = NULL; \
					break; \
				} \
			} \
			if (e && item_ptr) \
			{ \
				util_memcpy(item_ptr, &e->item, sizeof(ITEM_T)); \
				ret = true; \
			} \
		} \
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_GET_TAIL(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_get_tail(const LIST_T * const list, ITEM_T * const item_ptr) \
	{ \
		bool_t ret = false; \
		\
		if (list && list->tail) \
		{ \
			util_memcpy(item_ptr, &list->tail->item, sizeof(ITEM_T)); \
			ret = true; \
		} \
		\
	 	return ret; \
	}
#define UNBOUNDED_LIST_BODY_NEXT(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Return the next element, if one, or return NULL if there isn't
	 */ \
	PREFIX bool_t LIST_T##_next(const LIST_T * const list, const ITEM_T current, ITEM_T * const next_ptr) \
	{ \
		bool_t ret = false; \
		\
		LIST_T##_element_t * e = list->head; \
		for (uint32_t p = 0 ; p < list->size ; p++ ) \
		{ \
			if (e->item == current) \
			{ \
				break; \
			} \
			else \
			{ \
				e = e->next; \
			} \
		} \
		if (e->next) \
		{ \
			e = e->next; \
			if (next_ptr) \
			{ \
				util_memcpy(next_ptr, &e->item, sizeof(ITEM_T)); \
				ret = true; \
			} \
		} \
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_SIZE(PREFIX, LIST_T) \
	/**
	 * Get the size of the list
	 */ \
	PREFIX inline uint32_t LIST_T##_size(const LIST_T * const list) \
	{ \
		return list->size; \
	} \
	\

#endif /* UNBOUNDED_LIST_H_ */
