/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_LIST_ITERATOR_H_
#define UNBOUNDED_LIST_ITERATOR_H_

#include "tinker_api_types.h"
#include "../util_memcpy.h"
#include "kernel/memory/memory_manager.h"

#define UNBOUNDED_LIST_ITERATOR_TYPE(ITERATOR_T) \
	\
	typedef struct ITERATOR_T ITERATOR_T; \

#define UNBOUNDED_LIST_ITERATOR_INTERNAL_TYPE(ITERATOR_T, LIST_T, ITEM_T) \
	\
	typedef struct ITERATOR_T \
	{ \
		const LIST_T * list; \
		LIST_T##_element_t * current; \
	} ITERATOR_T##internal ; \

#define UNBOUNDED_LIST_ITERATOR_SPEC(PREFIX, ITERATOR_T, LIST_T, ITEM_T) \
	\
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, const LIST_T * const list); \
	\
	PREFIX ITERATOR_T * ITERATOR_T##_create(const LIST_T * const list); \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it); \
	\
	PREFIX bool_t ITERATOR_T##_get(ITERATOR_T * it, ITEM_T * item); \
	\
	PREFIX bool_t ITERATOR_T##_next(ITERATOR_T * it, ITEM_T * item); \
	\
	PREFIX void ITERATOR_T##_reset(ITERATOR_T * it); \
	\

#define UNBOUNDED_LIST_ITERATOR_BODY(PREFIX, ITERATOR_T, LIST_T, ITEM_T) \
	\
	/*
	 * Initialise a new linked list
	 */ \
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, const LIST_T * const list) \
	{ \
		 if ( it && list ) \
		 { \
			 it->list = list; \
			 if (list->size > 0) \
			 { \
				 it->current = list->head; \
			 } \
			 else \
			 { \
				 it->current = NULL; \
			 } \
		 } \
	} \
	\
	/*
	 * Create a new instance of an iterator for the linked list
	 */ \
	PREFIX ITERATOR_T * ITERATOR_T##_create(const LIST_T * const list) \
	{ \
		ITERATOR_T * it = NULL; \
		if ( list ) \
		{ \
			it = mem_alloc(list->pool, sizeof(ITERATOR_T)); \
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
			 mem_free(it->list->pool, it); \
		 } \
	} \
	\
	/*
	 * Get the current item from the list
	 */ \
	PREFIX bool_t ITERATOR_T##_get(ITERATOR_T * it, ITEM_T * item) \
	{ \
		bool_t ok = false; \
		if (it && item) \
		{ \
			if (!it->current) \
			{ \
				it->current = it->list->head; \
			} \
			\
			if (it->current) \
			{ \
				util_memcpy(item, &(it->current->item), sizeof(ITEM_T)); \
				ok = true; \
			} \
		} \
		\
		return ok; \
	} \
	/*
	 * Get the next element from the list
	 */ \
	PREFIX bool_t ITERATOR_T##_next(ITERATOR_T * it, ITEM_T * item) \
	{ \
		 bool_t ok = false; \
		 \
		 if (it && item) \
		 { \
			util_memset(item, 0, sizeof(ITEM_T)); \
			\
			 if ( it->current ) \
			 { \
				 if ( it->current->next ) \
				 { \
					 util_memcpy(item, &(it->current->next->item), sizeof(ITEM_T)); \
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
	extern inline void ITERATOR_T##_test(void) \
	{ \
		ITERATOR_T * item = ITERATOR_T##_create(NULL); \
		ITERATOR_T##_next(item, NULL); \
		ITERATOR_T##_reset(item); \
		ITERATOR_T##_delete(item); \
	} \
	\

#endif /* UNBOUNDED_LIST_ITERATOR_H_ */
