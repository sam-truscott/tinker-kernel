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
	\

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
	\

#define UNBOUNDED_QUEUE_SPEC_INITIALISE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_CREATE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	UNBOUNDED_LIST_SPEC_INITIALISE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX void QUEUE_T##_initialise(QUEUE_T * const queue, __mem_pool_info_t * const pool); \
	\

#define UNBOUNDED_QUEUE_SPEC_CREATE(PREFIX, QUEUE_T, ITEM_T) \
	PREFIX QUEUE_T * QUEUE_T##_create(__mem_pool_info_t * const pool); \
	\

#define UNBOUNDED_QUEUE_SPEC_DELETE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_DELETE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX void QUEUE_T##_delete(QUEUE_T * const queue); \
	\

#define UNBOUNDED_QUEUE_SPEC_PUSH(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_ADD(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_push(QUEUE_T * const queue, ITEM_T item); \
	\

#define UNBOUNDED_QUEUE_SPEC_POP(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_REMOVE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_pop(QUEUE_T * const queue) __attribute((used)); \
	\

#define UNBOUNDED_QUEUE_SPEC_FRONT(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_GET(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_front(const QUEUE_T * const queue, ITEM_T * const item_ptr); \
	\

#define UNBOUNDED_QUEUE_SPEC_REORDER_FIRST(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_HEAD_TO_TAIL(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_reorder_first(QUEUE_T * const queue); \
	\

#define UNBOUNDED_QUEUE_SPEC_SIZE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_SIZE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX uint32_t QUEUE_T##_size(const QUEUE_T * const queue); \
	\

#define UNBOUNDED_QUEUE_SPEC_REMOVE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC_REMOVE_ITEM(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_remove(QUEUE_T * const queue, ITEM_T item); \
	\

#define UNBOUNDED_QUEUE_BODY_INITIALISE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_CREATE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	UNBOUNDED_LIST_BODY_INITIALISE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX void QUEUE_T##_initialise(QUEUE_T * const queue, __mem_pool_info_t * const pool) \
	{ \
		if (queue) \
		{ \
			queue->list = QUEUE_T##_list_t_create(pool); \
		} \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_CREATE(PREFIX, QUEUE_T, ITEM_T) \
	PREFIX QUEUE_T * QUEUE_T##_create(__mem_pool_info_t * const pool) \
	{ \
		QUEUE_T * queue = NULL; \
		\
		if (pool) \
		{ \
			queue = (QUEUE_T*)__mem_alloc(pool, sizeof(QUEUE_T)); \
			if (queue) \
			{ \
				QUEUE_T##_initialise(queue, pool); \
			} \
		} \
		\
		return queue; \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_DELETE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_DELETE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX void QUEUE_T##_delete(QUEUE_T * const queue) \
	{ \
		if (queue) \
		{ \
			if (queue->list) \
			{ \
				__mem_pool_info_t * const pool = queue->list->pool; \
				QUEUE_T##_list_t_delete(queue->list);\
				__mem_free(pool, queue); \
			} \
		} \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_PUSH(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_ADD(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_push(QUEUE_T * const queue, ITEM_T item) \
	{ \
		bool_t ok = false; \
		if (queue && queue->list) \
		{ \
			ok = QUEUE_T##_list_t_add(queue->list, item); \
		} \
		return ok; \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_POP(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_REMOVE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_pop(QUEUE_T * const queue)\
	{ \
		bool_t ok = false; \
		if (queue && queue->list) \
		{ \
			ok = QUEUE_T##_list_t_remove(queue->list, 0); \
		} \
		return ok; \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_FRONT(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_GET(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_front(const QUEUE_T * const queue, ITEM_T * const item_ptr) \
	{ \
		bool_t ok = false; \
		if (queue && queue->list) \
		{ \
			ok = QUEUE_T##_list_t_get(queue->list, 0, item_ptr); \
		} \
		return ok; \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_REORDER_FIRST(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_HEAD_TO_TAIL(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_reorder_first(QUEUE_T * const queue) \
	{ \
		bool_t ok = false; \
		if (queue && queue->list) \
		{ \
			ok = QUEUE_T##_list_t_head_to_tail(queue->list); \
		} \
		\
		return ok; \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_SIZE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_SIZE(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX uint32_t QUEUE_T##_size(const QUEUE_T * const queue) \
	{ \
		uint32_t size = 0; \
		if (queue && queue->list) \
		{ \
			size = QUEUE_T##_list_t_size(queue->list); \
		} \
		return size; \
	} \
	\

#define UNBOUNDED_QUEUE_BODY_REMOVE(PREFIX, QUEUE_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY_REMOVE_ITEM(PREFIX, QUEUE_T##_list_t, ITEM_T) \
	\
	PREFIX bool_t QUEUE_T##_remove(QUEUE_T * const queue, ITEM_T item) \
	{ \
		bool_t ret = false; \
		\
		if (queue && queue->list) \
		{ \
			ret = QUEUE_T##_list_t_remove_item(queue->list, item); \
		} \
		\
		return ret; \
	} \
	\

#endif /* UNBOUNDED_QUEUE_H_ */
