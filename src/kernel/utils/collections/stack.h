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

#ifndef STACK_H_
#define STACK_H_

#include "sos3_api_types.h"
#include "../util_memset.h"
#include "kernel/memory/memory_manager.h"
#include "unbounded_list.h"

#define STACK_SPEC(PREFIX, STACK_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC(PREFIX, STACK_T##_list_t, ITEM_T) \
	\
	typedef struct \
	{ \
		STACK_T##_list_t * list; \
	} STACK_T; \
	\
	PREFIX void STACK_T##_initialise(STACK_T * stack, __mem_pool_info_t * pool); \
	\
	PREFIX STACK_T * STACK_T##_create(__mem_pool_info_t * pool); \
	\
	PREFIX void STACK_T##_delete(STACK_T * stack); \
	\
	PREFIX void STACK_T##_push(STACK_T * stack, ITEM_T item); \
	\
	PREFIX bool STACK_T##_pop(STACK_T * stack, ITEM_T * item); \
	\
	PREFIX bool STACK_T##_front(const STACK_T * stack, ITEM_T * item); \
	\
	PREFIX uint32_t STACK_T##_size(const STACK_T * stack); \
	\
	PREFIX bool STACK_T##_get(const STACK_T * stack, uint32_t index, ITEM_T * item); \
	\
	PREFIX bool STACK_T##_insert(STACK_T * stack, uint32_t index, ITEM_T item); \
	\

#define STACK_BODY(PREFIX, STACK_T, ITEM_T) \
	\
	UNBOUNDED_LIST_BODY(PREFIX, STACK_T##_list_t, ITEM_T) \
	\
	PREFIX void STACK_T##_initialise(STACK_T * stack, __mem_pool_info_t * pool) \
	{ \
		if ( stack ) \
		{ \
			stack->list = STACK_T##_list_t_create(pool); \
		} \
	} \
	\
	PREFIX STACK_T * STACK_T##_create(__mem_pool_info_t * pool) \
	{ \
		STACK_T * stack = NULL; \
		if ( pool ) \
		{ \
			stack = (STACK_T*)__mem_alloc(pool, sizeof(STACK_T)); \
			STACK_T##_initialise(stack, pool); \
		} \
		return stack; \
	} \
	\
	PREFIX void STACK_T##_delete(STACK_T * stack) \
	{ \
		if ( stack ) \
		{ \
			if ( stack->list ) \
			{ \
				STACK_T##_list_t_delete(stack->list);\
			} \
			/* FIXME This'll fail as pool will be NULL */ \
			__mem_free(stack->list->pool, stack); \
		} \
	} \
	\
	PREFIX void STACK_T##_push(STACK_T * stack, ITEM_T item) \
	{ \
		if ( stack ) \
		{ \
			STACK_T##_list_t_add(stack->list, item); \
		} \
	} \
	PREFIX bool STACK_T##_insert(STACK_T * stack, uint32_t index, ITEM_T item) \
	{ \
		bool ok = false; \
		\
		if ( stack ) \
		{ \
			ok = STACK_T##_list_t_insert(stack->list, index, item); \
		} \
		return ok; \
	} \
	\
	PREFIX bool STACK_T##_pop(STACK_T * stack, ITEM_T * item) \
	{ \
		bool ok = false; \
		\
		if ( stack && item ) \
		{ \
			uint32_t index = STACK_T##_list_t_size(stack->list) - 1; \
			if ( (ok = STACK_T##_list_t_get(stack->list, index, item)) ) \
			{ \
				STACK_T##_list_t_remove(stack->list, index); \
			} \
		} \
		\
		return ok; \
	} \
	\
	PREFIX bool STACK_T##_front(const STACK_T * stack, ITEM_T * item) \
	{ \
		bool ok = false; \
		\
		if ( stack && item ) \
		{ \
			const uint32_t index = STACK_T##_list_t_size(stack->list) - 1; \
			ok = STACK_T##_list_t_get(stack->list, index, item); \
		} \
		\
		return ok; \
	} \
	\
	PREFIX bool STACK_T##_get(const STACK_T * stack, uint32_t index, ITEM_T * item) \
	{ \
		bool ok = false; \
		\
		if ( stack && item ) \
		{ \
			ok = STACK_T##_list_t_get(stack->list, index, item); \
		} \
		\
		return ok; \
	} \
	\
	PREFIX uint32_t STACK_T##_size(const STACK_T * stack) \
	{ \
		uint32_t size = 0; \
		\
		if ( stack && stack->list ) \
		{ \
			size = STACK_T##_list_t_size(stack->list); \
		} \
		\
		return size; \
	} \
	\
	extern void STACK##_unused__(void) \
	{ \
		STACK_T##_create(NULL); \
		STACK_T##_delete(NULL); \
		STACK_T##_push(NULL, NULL); \
		STACK_T##_pop(NULL, NULL); \
		STACK_T##_size(NULL); \
	} \
	\

#endif /* STACK_H_ */
