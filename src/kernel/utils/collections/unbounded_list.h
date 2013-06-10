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
#define UNBOUNDED_LIST_SPEC(PREFIX, LIST_T, ITEM_T) \
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
	typedef struct LIST_T##_STRUCT \
	{ \
		uint32_t size; \
		LIST_T##_element_t * head; \
		LIST_T##_element_t * tail; \
		__mem_pool_info_t * pool; \
	} LIST_T; \
	\
	PREFIX void LIST_T##_initialise(LIST_T * list, __mem_pool_info_t * pool); \
	\
	PREFIX LIST_T * LIST_T##_create(__mem_pool_info_t * pool); \
	\
	PREFIX void LIST_T##_delete(LIST_T * list); \
	\
	PREFIX bool LIST_T##_add(LIST_T * list, ITEM_T item); \
	\
	PREFIX bool LIST_T##_insert(LIST_T * list, uint32_t index, ITEM_T item); \
	\
	PREFIX bool LIST_T##_remove(LIST_T * list, const uint32_t index); \
	\
	PREFIX bool LIST_T##_remove_item(LIST_T * list, ITEM_T item); \
	\
	PREFIX bool LIST_T##_get(const LIST_T * list, const uint32_t index, ITEM_T * item_ptr); \
	\
	PREFIX bool LIST_T##_next(const LIST_T * list, const ITEM_T current, ITEM_T * next_ptr); \
	\
	PREFIX uint32_t LIST_T##_size(const LIST_T * list); \
	\
	PREFIX uint32_t LIST_T##_next_index(const LIST_T * list); \
	\

#define UNBOUNDED_LIST_BODY(PREFIX, LIST_T, ITEM_T) \
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
	} \
	\
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
	} \
	\
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
	} \
	\
	/**
	 * Add an entry to the list
	 */ \
	PREFIX bool LIST_T##_add(LIST_T * list, ITEM_T item) \
	{ \
		bool ret = false; \
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
	} \
	\
	PREFIX bool LIST_T##_insert(LIST_T * list, uint32_t index, ITEM_T item) \
	{ \
		bool ok = false; \
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
	} \
	\
	/**
	 * Delete an entry from the list
	 */ \
	PREFIX bool LIST_T##_remove(LIST_T * list, const uint32_t index) \
	{ \
		bool ret = false; \
		\
		if ( list ) \
		{ \
			if ( list->size > index ) \
			{ \
				LIST_T##_element_t * e = list->head; \
				LIST_T##_element_t * p = NULL; \
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
	} \
	\
	/**
	 * Remove an item from the list
	 */ \
	PREFIX bool LIST_T##_remove_item(LIST_T * list, ITEM_T item) \
	{ \
		bool ret = false; \
		\
		if ( list->size ) \
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
	} \
	/**
	 * Get an entry from the list
	 */ \
	PREFIX bool LIST_T##_get(const LIST_T * list, const uint32_t index, ITEM_T * item_ptr) \
	{ \
		bool ret = false; \
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
	} \
	\
	/**
	 * Return the next element, if one, or return NULL if there isn't
	 */ \
	PREFIX bool LIST_T##_next(const LIST_T * list, const ITEM_T current, ITEM_T * next_ptr) \
	{ \
		bool ret = false; \
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
	} \
	/**
	 * Get the size of the list
	 */ \
	PREFIX uint32_t LIST_T##_size(const LIST_T * list) \
	{ \
		return list->size; \
	} \
	\
	/**
	 * Get the next free index
	 */ \
	\
	PREFIX uint32_t LIST_T##_next_index(const LIST_T * list) \
	{ \
		return LIST_T##_size(list); \
	} \
	\
	extern void LIST_T##_test__(void) \
	{ \
		LIST_T##_create(NULL); \
		LIST_T##_delete(NULL); \
		LIST_T##_next(NULL, NULL, NULL); \
		LIST_T##_next_index(NULL); \
		LIST_T##_remove_item(NULL, NULL); \
		LIST_T##_insert(NULL, 0, NULL); \
	} \

#endif /* UNBOUNDED_LIST_H_ */
