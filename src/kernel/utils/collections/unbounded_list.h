/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_LIST_H_
#define UNBOUNDED_LIST_H_

#include "sos_api_types.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/memory/memory_manager.h"

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
		__mem_pool_info_t * pool; \
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
	PREFIX void LIST_T##_initialise(LIST_T * list, __mem_pool_info_t * pool);
#define UNBOUNDED_LIST_SPEC_CREATE(PREFIX, LIST_T, ITEM_T) \
	PREFIX LIST_T * LIST_T##_create(__mem_pool_info_t * pool);
#define UNBOUNDED_LIST_SPEC_DELETE(PREFIX, LIST_T, ITEM_T) \
	PREFIX void LIST_T##_delete(LIST_T * list);
#define UNBOUNDED_LIST_SPEC_ADD(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_add(LIST_T * list, ITEM_T item);
#define UNBOUNDED_LIST_SPEC_INSERT(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_insert(LIST_T * list, uint32_t index, ITEM_T item);
#define UNBOUNDED_LIST_SPEC_REMOVE(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_remove(LIST_T * list, const uint32_t index);
#define UNBOUNDED_LIST_SPEC_REMOVE_ITEM(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_remove_item(LIST_T * list, ITEM_T item);
#define UNBOUNDED_LIST_SPEC_HEAD_TO_TAIL(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_head_to_tail(LIST_T * list);
#define UNBOUNDED_LIST_SPEC_GET(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_get(const LIST_T * list, const uint32_t index, ITEM_T * item_ptr);
#define UNBOUNDED_LIST_SPEC_NEXT(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_next(const LIST_T * list, const ITEM_T current, ITEM_T * next_ptr);
#define UNBOUNDED_LIST_SPEC_SIZE(PREFIX, LIST_T, ITEM_T) \
	PREFIX inline uint32_t LIST_T##_size(const LIST_T * list);

#define UNBOUNDED_LIST_BODY_INITIALISE(PREFIX, LIST_T, ITEM_T) \
	\
	/**
	 * Initialise the data structure
	 */ \
	PREFIX void LIST_T##_initialise(LIST_T * list, __mem_pool_info_t * pool) \
	{ \
		 if ( list != NULL ) \
		 { \
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
	PREFIX LIST_T * LIST_T##_create(__mem_pool_info_t * pool) \
	{ \
		LIST_T * lst = NULL; \
		lst = __mem_alloc(pool, sizeof(LIST_T)); \
		if ( lst != NULL ) \
		{ \
			LIST_T##_initialise(lst, pool); \
		} \
		return lst; \
	}
#define UNBOUNDED_LIST_BODY_DELETE(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Delete the list and every element
	 */ \
	PREFIX void LIST_T##_delete(LIST_T * list) \
	{ \
		if ( list && list->pool ) \
		{ \
			while(list->size > 0) \
			{ \
				LIST_T##_element_t * e = list->head; \
				list->head = e->next; \
				__mem_free( list->pool, e ); \
				list->size--; \
			} \
			__mem_free( list-> pool, list ); \
		} \
	}
#define UNBOUNDED_LIST_BODY_ADD(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Add an entry to the list
	 */ \
	PREFIX bool_t LIST_T##_add(LIST_T * list, ITEM_T item) \
	{ \
		bool_t ret = false; \
		\
		if ( list ) \
		{ \
			LIST_T##_element_t * element = __mem_alloc( \
					list->pool, sizeof(LIST_T##_element_t)); \
			\
			/* initialise the new item */ \
			if ( element != NULL ) \
			{ \
				element->item = item; \
				element->next = NULL; \
				if ( list->tail == NULL ) \
				{ \
					list->head = list->tail = element; \
				} else { \
					list->tail->next = element; \
					list->tail = element; \
				} \
				list->size++; \
				ret = true; \
			} \
		} \
		\
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_INSERT(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_insert(LIST_T * list, uint32_t index, ITEM_T item) \
	{ \
		bool_t ok = false; \
		\
		if ( list ) \
		{ \
			LIST_T##_element_t * element = __mem_alloc( \
					list->pool, sizeof(LIST_T##_element_t)); \
			\
			if ( element ) \
			{ \
				/* get the head of the list */ \
				LIST_T##_element_t * e = list->head; \
				uint32_t p = 0; \
				\
				/* setup our new element */ \
				element->item = item; \
				element->next = NULL; \
				\
				if ( index ) \
				{ \
					/* get the index we need */ \
					for ( ; p < index - 1; p++ ) \
					{ \
						e = e->next; \
					} \
				} \
				/* setup our pointers correctly */ \
				if ( e ) \
				{ \
					if ( e->next ) \
					{ \
						element->next = e->next; \
					} \
					\
					if ( e == list->tail ) \
					{ \
						list->tail = element; \
					} \
					\
					e->next = element; \
					\
					ok = true; \
					list->size++; \
				} \
			} \
		} \
		return ok; \
	}
#define UNBOUNDED_LIST_BODY_REMOVE(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Delete an entry from the list
	 */ \
	PREFIX bool_t LIST_T##_remove(LIST_T * list, const uint32_t index) \
	{ \
		bool_t ret = false; \
		\
		if (list) \
		{ \
			if (list->size > index) \
			{ \
				LIST_T##_element_t * e = list->head; \
				LIST_T##_element_t * p = e; \
				uint32_t c = 0; \
				/*
				 * find the right element, keeping track
				 * of the previous one
				 */ \
				for ( ; c < index ; c++ ) \
				{ \
					p = e; \
					e = e->next; \
				} \
				/*
				 * if the previous node isnt this node we
				 * need to update the previous node
				 */ \
				if ( p && p != e) \
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
					} else { \
						p->next = NULL; \
					} \
				} \
				\
				/*
				 * Update the tail of the list
				 */ \
				if ( list->size == 1 ) \
				{ \
					list->tail = list->head = NULL; \
				} \
				else \
				{ \
					if ( e == list->tail ) \
					{ \
						list->tail = p; \
					} \
					if ( e == list->head) \
					{ \
						list->head = e->next; \
					} \
				} \
				\
				list->size--; \
				__mem_free(list->pool, e); \
				ret = true; \
			} \
		} \
		\
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_REMOVE_ITEM(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Remove an item from the list
	 */ \
	PREFIX bool_t LIST_T##_remove_item(LIST_T * list, ITEM_T item) \
	{ \
		bool_t ret = false; \
		\
		if ( list && list->size ) \
		{ \
			LIST_T##_element_t * e = list->head; \
			uint32_t p = 0; \
			for ( ; p < list->size ; p++ ) \
			{ \
				if ( e->item == item ) \
				{ \
					ret = LIST_T##_remove(list, p); \
					break; \
				} \
				e = e->next; \
			} \
		} \
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_HEAD_TO_TAIL(PREFIX, LIST_T, ITEM_T) \
	PREFIX bool_t LIST_T##_head_to_tail(LIST_T * list) \
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
				LIST_T##_element_t * oldHead = list->head; \
				LIST_T##_element_t * oldTail = list->tail; \
				list->head = oldHead->next; \
				oldHead->next = NULL; \
				oldTail->next = oldHead; \
				list->tail = oldHead; \
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
	PREFIX bool_t LIST_T##_get(const LIST_T * list, const uint32_t index, ITEM_T * item_ptr) \
	{ \
		bool_t ret = false; \
		\
		if ( list->size > index ) \
		{ \
			LIST_T##_element_t * e = list->head; \
			uint32_t p = 0; \
			for ( ; p < index ; p++ ) \
			{ \
				e = e->next; \
			} \
			if ( item_ptr != NULL ) \
			{ \
				__util_memcpy(item_ptr, &e->item, sizeof(ITEM_T)); \
				ret = true; \
			} \
		} \
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_NEXT(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Return the next element, if one, or return NULL if there isn't
	 */ \
	PREFIX bool_t LIST_T##_next(const LIST_T * list, const ITEM_T current, ITEM_T * next_ptr) \
	{ \
		bool_t ret = false; \
		\
		LIST_T##_element_t * e = list->head; \
		uint32_t p = 0; \
		for ( ; p < list->size ; p++ ) \
		{ \
			if ( e->item == current ) \
			{ \
				break; \
			} \
			else \
			{ \
				e = e->next; \
			} \
		} \
		if ( e->next ) \
		{ \
			e = e->next; \
			if ( next_ptr != NULL ) \
			{ \
				__util_memcpy(next_ptr, &e->item, sizeof(ITEM_T)); \
				ret = true; \
			} \
		} \
		return ret; \
	}
#define UNBOUNDED_LIST_BODY_SIZE(PREFIX, LIST_T, ITEM_T) \
	/**
	 * Get the size of the list
	 */ \
	PREFIX inline uint32_t LIST_T##_size(const LIST_T * list) \
	{ \
		return list->size; \
	} \
	\

#endif /* UNBOUNDED_LIST_H_ */
