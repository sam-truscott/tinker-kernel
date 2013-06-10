/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef UNBOUNDED_LIST_ITERATOR_H_
#define UNBOUNDED_LIST_ITERATOR_H_

#include "sos3_api_types.h"
#include "../util_memcpy.h"
#include "kernel/memory/memory_manager.h"

#define UNBOUNDED_LIST_ITERATOR_SPEC(PREFIX, ITERATOR_T, LIST_T, ITEM_T) \
	\
	typedef struct ITERATOR_T##_STRUCT \
	{ \
		LIST_T * list; \
		LIST_T##_element_t * current; \
	} ITERATOR_T ; \
	\
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, LIST_T * list); \
	\
	PREFIX ITERATOR_T * ITERATOR_T##_create(LIST_T * list); \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it); \
	\
	PREFIX bool ITERATOR_T##_get(ITERATOR_T * it, ITEM_T * item); \
	\
	PREFIX bool ITERATOR_T##_next(ITERATOR_T * it, ITEM_T * item); \
	\
	PREFIX void ITERATOR_T##_reset(ITERATOR_T * it); \
	\

#define UNBOUNDED_LIST_ITERATOR_BODY(PREFIX, ITERATOR_T, LIST_T, ITEM_T) \
	\
	/*
	 * Initialise a new linked list
	 */ \
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, LIST_T * list) \
	{ \
		 if ( it && list ) \
		 { \
			 it->list = list; \
			 if ( list->size > 0 ) \
			 { \
				 it->current = list->head; \
			 } else { \
				 it->current = NULL; \
			 } \
		 } \
	} \
	\
	/*
	 * Create a new instance of an iterator for the linked list
	 */ \
	PREFIX ITERATOR_T * ITERATOR_T##_create(LIST_T * list) \
	{ \
		ITERATOR_T * it = NULL; \
		if ( list ) \
		{ \
			it = __mem_alloc(list->pool, sizeof(ITERATOR_T)); \
			ITERATOR_T##_initialise(it, list); \
		} \
		return it;\
	} \
	\
	/*
	 * Delete the iterator
	 */ \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it) \
	{ \
		 if ( it ) \
		 { \
			 __mem_free(it->list->pool, it); \
		 } \
	} \
	\
	/*
	 * Get the current item from the list
	 */ \
	PREFIX bool ITERATOR_T##_get(ITERATOR_T * it, ITEM_T * item) \
	{ \
		bool ok = false; \
		if ( it && item ) \
		{ \
			if ( !it->current ) \
			{ \
				it->current = it->list->head; \
			} \
			\
			if ( it->current ) \
			{ \
				__util_memcpy(item, &(it->current->item), sizeof(ITEM_T)); \
				ok = true; \
			} \
		} \
		\
		return ok; \
	} \
	/*
	 * Get the next element from the list
	 */ \
	PREFIX bool ITERATOR_T##_next(ITERATOR_T * it, ITEM_T * item) \
	{ \
		 bool ok = false; \
		 \
		 if ( it && item ) \
		 { \
			\
			 if ( it->current ) \
			 { \
				 if ( it->current->next ) \
				 { \
					 __util_memcpy(item, &(it->current->next->item), sizeof(ITEM_T)); \
					 it->current = it->current->next; \
					 ok = true; \
				 } \
			 } \
		 } \
		 \
		 return ok; \
	} \
	\
	PREFIX void ITERATOR_T##_reset(ITERATOR_T * it) \
	{ \
		if ( it ) \
		{ \
			it->current = it->list->head; \
		} \
	} \
	\
	extern inline void ITERATOR_T##_test__(void) \
	{ \
		ITERATOR_T * item = ITERATOR_T##_create(NULL); \
		ITERATOR_T##_next(item, NULL); \
		ITERATOR_T##_delete(item); \
	} \
	\

#endif /* UNBOUNDED_LIST_ITERATOR_H_ */
