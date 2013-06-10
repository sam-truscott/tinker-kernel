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

#ifndef BOUNDED_VALUE_ARRAY_H_
#define BOUNDED_VALUE_ARRAY_H_

#include "sos3_api_types.h"

/*
 * The default size of the array
 */
#define DEFAULT_ARRAY_CAPACITY 10u

/*
 * Redirect the default one (10 items) to the bounded one
 */
#define DEFAULT_BOUNDED_ARRAY_T(ARRAY_T, ITEM_T) \
	BOUNDED_ARRAY_T(ARRAY_T, ITEM_T, DEFAULT_ARRAY_CAPACITY)

/*
 * Macro for the bound value (static) array
 */
#define BOUNDED_ARRAY_SPEC(PREFIX, ARRAY_T, ITEM_T, SIZE) \
	\
	/*
	 * The data structure to use for the array
	 */ \
	typedef struct ARRAY_T##_STRUCT \
	{ \
		uint32_t capacity; \
		ITEM_T data[SIZE]; \
		__mem_pool_info_t * pool; \
	} ARRAY_T; \
	\
	PREFIX ARRAY_T * ARRAY_T##_create(__mem_pool_info_t * pool); \
	\
	PREFIX void ARRAY_T##_delete(ARRAY_T * arr); \
	\
	PREFIX ITEM_T ARRAY_T##_get(ARRAY_T* arr, uint32_t index); \
	\
	PREFIX void ARRAY_T##_set(ARRAY_T* arr, uint32_t index, ITEM_T value); \
	\
	PREFIX uint32_t ARRAY_T##_capacity(ARRAY_T* arr); \
	\

#define BOUNDED_ARRAY_BODY(PREFIX, ARRAY_T, ITEM_T, SIZE) \
	/*
	 *
	 */ \
	PREFIX void ARRAY_T##_initialise(ARRAY_T * arr, __mem_pool_info_t * pool) \
	{ \
		 if ( arr != NULL ) \
		 { \
			 arr->capacity = SIZE; \
			 arr->pool = pool; \
		 } \
	} \
	\
	/*
	 * Constructor
	 */ \
	PREFIX ARRAY_T * ARRAY_T##_create(__mem_pool_info_t * pool) \
	{ \
		ARRAY_T * arr = NULL; \
		arr = __mem_alloc(pool, sizeof(ARRAY_T)); \
		ARRAY_T##_initialise(arr, pool); \
		return arr; \
	} \
	\
	/*
	 * Delete the array
	 */ \
	\
	PREFIX void ARRAY_T##_delete(ARRAY_T * arr) \
	{ \
		if ( arr != NULL && arr->pool != NULL ) \
		{ \
			__mem_free( arr->pool, arr ); \
		} \
	} \
	/*
	 * Get an element
	 */ \
	PREFIX ITEM_T ARRAY_T##_get(ARRAY_T* arr, uint32_t index) \
	{ \
		return arr->data[index]; \
	} \
	\
	/*
	 * Set an element
	 */ \
	PREFIX void ARRAY_T##_set(ARRAY_T* arr, uint32_t index, ITEM_T value) \
	{ \
		arr->data[index] = value; \
	} \
	\
	/*
	 * Determine the size of the array
	 */ \
	PREFIX uint32_t ARRAY_T##_capacity(ARRAY_T* arr) \
	{ \
		 return arr->capacity; \
	} \
	\
	/*
	 * Test routine
	 */ \
	extern inline void ARRAY_T##_test__(void) \
	{ \
		ARRAY_T * arr = ARRAY_T##_create(NULL); \
		ARRAY_T##_capacity(arr); \
		ARRAY_T##_delete(arr); \
	} \

#endif /* BOUNDED_ARRAY_H_ */
