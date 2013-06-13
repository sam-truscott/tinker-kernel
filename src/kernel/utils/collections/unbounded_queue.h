/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_QUEUE_H_
#define UNBOUNDED_QUEUE_H_

#include "sos_api_types.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/memory/memory_manager.h"
#include "unbounded_list.h"

#define UNBOUNDED_QUEUE_TYPE(QUEUE_T) \
	\
	typedef struct QUEUE_T QUEUE_T; \

#define UNBOUNDED_QUEUE_INTERNAL_TYPE(QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_TYPE(QUEUE_T##_list_t) \
	UNBOUNDED_LIST_INTERNAL_TYPE(QUEUE_T##_list_t, ITEM_T) \
	\
	typedef struct QUEUE_T \
	{ \
		QUEUE_T##_list_t * list; \
		\
	} QUEUE_T##_internal; \

#define UNBOUNDED_QUEUE_SPEC(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX void QUEUE_T##_initialise(QUEUE_T * queue, __mem_pool_info_t * pool); \
	\
	PREFIX QUEUE_T * QUEUE_T##_create(__mem_pool_info_t * pool); \
	\
	PREFIX void QUEUE_T##_delete(QUEUE_T * queue); \
	\
	PREFIX bool QUEUE_T##_push(QUEUE_T * queue, ITEM_T item); \
	\
	PREFIX bool QUEUE_T##_pop(QUEUE_T * queue); \
	\
	PREFIX bool QUEUE_T##_front(const QUEUE_T * queue, ITEM_T * item_ptr); \
	\
	PREFIX bool QUEUE_T##_reorder_first(QUEUE_T * queue); \
	\
	PREFIX uint32_t QUEUE_T##_size(const QUEUE_T * queue); \
	\
	PREFIX bool QUEUE_T##_remove(QUEUE_T * queue, ITEM_T item); \
	\

#define UNBOUNDED_QUEUE_BODY(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX void QUEUE_T##_initialise(QUEUE_T * queue, __mem_pool_info_t * pool) \
	{ \
		if ( queue ) \
		{ \
			queue->list = QUEUE_T##_list_t_create(pool); \
		} \
	} \
	\
	PREFIX QUEUE_T * QUEUE_T##_create(__mem_pool_info_t * pool) \
	{ \
		QUEUE_T * queue = NULL; \
		\
		if ( pool ) \
		{ \
			queue = (QUEUE_T*)__mem_alloc(pool, sizeof(QUEUE_T)); \
			if ( queue ) \
			{ \
				QUEUE_T##_initialise(queue, pool); \
			} \
		} \
		\
		return queue; \
	} \
	\
	PREFIX void QUEUE_T##_delete(QUEUE_T * queue) \
	{ \
		if ( queue ) \
		{ \
			if ( queue->list ) \
			{ \
				QUEUE_T##_list_t_delete(queue->list);\
			} \
			/* FIXME This'll fail as pool will be NULL */ \
			__mem_free(queue->list->pool, queue); \
		} \
	} \
	\
	PREFIX bool QUEUE_T##_push(QUEUE_T * queue, ITEM_T item) \
	{ \
		bool ok = false; \
		if ( queue && queue->list ) \
		{ \
			ok = QUEUE_T##_list_t_add(queue->list, item); \
		} \
		return ok; \
	} \
	\
	PREFIX bool QUEUE_T##_pop(QUEUE_T * queue) \
	{ \
		bool ok = false; \
		if ( queue && queue->list ) \
		{ \
			ok = QUEUE_T##_list_t_remove(queue->list, 0); \
		} \
		return ok; \
	} \
	\
	PREFIX bool QUEUE_T##_front(const QUEUE_T * queue, ITEM_T * item_ptr) \
	{ \
		bool ok = false; \
		if ( queue && queue->list ) \
		{ \
			ok = QUEUE_T##_list_t_get(queue->list, 0, item_ptr); \
		} \
		return ok; \
	} \
	\
	PREFIX bool QUEUE_T##_reorder_first(QUEUE_T * queue) \
	{ \
		bool ok = false; \
		if (queue && queue->list) \
		{ \
			ok = QUEUE_T##_list_t_head_to_tail(queue->list); \
		} \
		\
		return ok; \
	} \
	\
	PREFIX uint32_t QUEUE_T##_size(const QUEUE_T * queue) \
	{ \
		uint32_t size = 0; \
		if ( queue && queue->list ) \
		{ \
			size = QUEUE_T##_list_t_size(queue->list); \
		} \
		return size; \
	} \
	\
	PREFIX bool QUEUE_T##_remove(QUEUE_T * queue, ITEM_T item) \
	{ \
		bool ret = false; \
		\
		if ( queue && queue->list ) \
		{ \
			ret = QUEUE_T##_list_t_remove_item(queue->list, item); \
		} \
		\
		return ret; \
	} \
	\
	extern void QUEUE_T##_unused__(void) \
	{ \
		QUEUE_T##_create(NULL); \
		QUEUE_T##_delete(NULL); \
		QUEUE_T##_push(NULL, NULL); \
		QUEUE_T##_pop(NULL); \
		QUEUE_T##_front(NULL, NULL); \
		QUEUE_T##_remove(NULL, NULL); \
		QUEUE_T##_size(NULL); \
		QUEUE_T##_reorder_first(NULL); \
	} \

#endif /* UNBOUNDED_QUEUE_H_ */
