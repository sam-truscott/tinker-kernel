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

#ifndef HASHED_MAP_ITERATOR_H_
#define HASHED_MAP_ITERATOR_H_

#include "sos3_api_types.h"
#include "../util_memcpy.h"
#include "kernel/memory/memory_manager.h"

#define HASH_MAP_TYPE_ITERATOR_SPEC(PREFIX, ITERATOR_T, HASH_MAP_T, VALUE_T) \
	\
	typedef struct ITERATOR_T##_STRUCT \
	{ \
		HASH_MAP_T * map; \
		HASH_MAP_T##entry_t * map_entry; \
		uint32_t position; \
	} ITERATOR_T;\
	\
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, HASH_MAP_T * map); \
	\
	PREFIX ITERATOR_T * ITERATOR_T##_create(HASH_MAP_T * map); \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it); \
	\
	PREFIX bool ITERATOR_T##_get(ITERATOR_T * it, VALUE_T * item); \
	\
	PREFIX bool ITERATOR_T##_next(ITERATOR_T * it, VALUE_T * item); \
	\
	PREFIX void ITERATOR_T##_reset(ITERATOR_T * it); \
	\

#define HASH_MAP_TYPE_ITERATOR_BODY(PREFIX, ITERATOR_T, HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY)\
	\
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, HASH_MAP_T * map) \
	{ \
		if ( it && map ) \
		{ \
			it->map = map; \
			for ( uint32_t i = 0 ; i < MAP_CAPACITY ; i++ ) \
			{ \
				if ( it->map->table[i].used ) \
				{ \
					it->map_entry = &it->map->table[i]; \
					it->position = i; \
					break; \
				} \
			} \
		} \
	} \
	\
	PREFIX ITERATOR_T * ITERATOR_T##_create(HASH_MAP_T * map) \
	{ \
		ITERATOR_T * it = NULL; \
		if ( map ) \
		{ \
			it = __mem_alloc(map->pool, sizeof(ITERATOR_T)); \
			ITERATOR_T##_initialise(it, map); \
		} \
		return it;\
	} \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it) \
	{ \
		if ( it ) \
		{ \
			__mem_free(it->map->pool, it); \
		} \
	} \
	\
	PREFIX bool ITERATOR_T##_get(ITERATOR_T * it, VALUE_T * item) \
	{ \
		bool ok = false; \
		\
		if ( it && item ) \
		{ \
			if ( it->map_entry ) \
			{ \
				__util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T)); \
				ok = true;\
			} \
		} \
		\
		return ok; \
	} \
	\
	PREFIX bool ITERATOR_T##_next(ITERATOR_T * it, VALUE_T * item) \
	{ \
		bool ok = false; \
		\
		if ( it && item ) \
		{ \
			for ( uint32_t i = it->position + 1 ; i < MAP_CAPACITY ; i++ ) \
			{ \
				if ( it->map->table[i].used ) \
				{ \
					it->position = i; \
					it->map_entry = &it->map->table[i]; \
					__util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T)); \
					ok = true; \
					break; \
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
			for ( uint32_t i = 0 ; i < MAP_CAPACITY ; i++ ) \
			{ \
				if ( it->map->table[i].used ) \
				{ \
					it->map_entry = &it->map->table[i]; \
					it->position = i; \
				} \
			} \
		} \
	} \
	\
	extern inline void ITERATOR_T##_test__(void) \
	{ \
		ITERATOR_T * item = ITERATOR_T##_create(NULL); \
		ITERATOR_T##_next(item, NULL); \
		ITERATOR_T##_get(item, NULL); \
		ITERATOR_T##_reset(item); \
		ITERATOR_T##_delete(item); \
		ITERATOR_T##_test__(); \
	} \

#endif /* HASHED_MAP_ITERATOR_H_ */
