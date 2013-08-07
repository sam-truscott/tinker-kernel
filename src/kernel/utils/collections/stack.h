/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef STACK_H_
#define STACK_H_

#include "sos_api_types.h"
#include "../util_memset.h"
#include "kernel/memory/memory_manager.h"
#include "unbounded_list.h"

#define STACK_TYPE(STACK_T) \
	\
	typedef struct STACK_T STACK_T; \

#define STACK_INTERNAL_TYPE(STACK_T, ITEM_T) \
	\
	UNBOUNDED_LIST_TYPE(STACK_T##_list_t) \
	UNBOUNDED_LIST_INTERNAL_TYPE(STACK_T##_list_t, ITEM_T) \
	\
	typedef struct STACK_T\
	{ \
		STACK_T##_list_t * list; \
	} STACK_T##__internal; \
	\

#define STACK_SPEC(PREFIX, STACK_T, ITEM_T) \
	\
	UNBOUNDED_LIST_SPEC(PREFIX, STACK_T##_list_t, ITEM_T) \
	\
	PREFIX void STACK_T##_initialise(STACK_T * stack, __mem_pool_info_t * pool); \
	\
	PREFIX STACK_T * STACK_T##_create(__mem_pool_info_t * pool); \
	\
	PREFIX void STACK_T##_delete(STACK_T * stack); \
	\
	PREFIX void STACK_T##_push(STACK_T * stack, ITEM_T item); \
	\
	PREFIX bool_t STACK_T##_pop(STACK_T * stack, ITEM_T * item); \
	\
	PREFIX bool_t STACK_T##_front(const STACK_T * stack, ITEM_T * item); \
	\
	PREFIX uint32_t STACK_T##_size(const STACK_T * stack); \
	\
	PREFIX bool_t STACK_T##_get(const STACK_T * stack, uint32_t index, ITEM_T * item); \
	\
	PREFIX bool_t STACK_T##_insert(STACK_T * stack, uint32_t index, ITEM_T item); \
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
	PREFIX bool_t STACK_T##_insert(STACK_T * stack, uint32_t index, ITEM_T item) \
	{ \
		bool_t ok = false; \
		\
		if ( stack ) \
		{ \
			ok = STACK_T##_list_t_insert(stack->list, index, item); \
		} \
		return ok; \
	} \
	\
	PREFIX bool_t STACK_T##_pop(STACK_T * stack, ITEM_T * item) \
	{ \
		bool_t ok = false; \
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
	PREFIX bool_t STACK_T##_front(const STACK_T * stack, ITEM_T * item) \
	{ \
		bool_t ok = false; \
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
	PREFIX bool_t STACK_T##_get(const STACK_T * stack, uint32_t index, ITEM_T * item) \
	{ \
		bool_t ok = false; \
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
