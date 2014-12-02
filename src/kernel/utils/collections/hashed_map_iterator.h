/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef HASHED_MAP_ITERATOR_H_
#define HASHED_MAP_ITERATOR_H_

#include "tinker_api_types.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/memory/memory_manager.h"

#define HASH_MAP_TYPE_ITERATOR_TYPE(ITERATOR_T) \
	\
	typedef struct ITERATOR_T ITERATOR_T;

#define HASH_MAP_TYPE_ITERATOR_INTERNAL_TYPE(ITERATOR_T, HASH_MAP_T) \
	\
	typedef struct ITERATOR_T \
	{ \
		HASH_MAP_T * map; \
		HASH_MAP_T##_entry_t * map_entry; \
		HASH_MAP_T##_bucket_t * map_bucket; \
		uint32_t bucket; \
		uint32_t entry; \
	} ITERATOR_T##_internal_t;\

#define HASH_MAP_TYPE_ITERATOR_SPEC(PREFIX, ITERATOR_T, VALUE_T) \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it); \
	\
	PREFIX bool_t ITERATOR_T##_get(ITERATOR_T * it, VALUE_T * item); \
	\
	PREFIX bool_t ITERATOR_T##_next(ITERATOR_T * it, VALUE_T * item); \
	\
	PREFIX void ITERATOR_T##_reset(ITERATOR_T * it); \
	\

#define HASH_MAP_TYPE_ITERATOR_BODY(PREFIX, ITERATOR_T, HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY, BUCKET_SIZE)\
	\
	PREFIX ITERATOR_T * ITERATOR_T##_create(HASH_MAP_T * map); \
	\
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, HASH_MAP_T * map); \
	\
	PREFIX void ITERATOR_T##_initialise(ITERATOR_T * it, HASH_MAP_T * map) \
	{ \
		if (it && map) \
		{ \
			it->map = map; \
			ITERATOR_T##_reset(it); \
		} \
	} \
	\
	PREFIX ITERATOR_T * ITERATOR_T##_create(HASH_MAP_T * map) \
	{ \
		ITERATOR_T * it = NULL; \
		if (map) \
		{ \
			it = mem_alloc(map->pool, sizeof(ITERATOR_T)); \
			ITERATOR_T##_initialise(it, map); \
		} \
		return it;\
	} \
	\
	PREFIX void ITERATOR_T##_delete(ITERATOR_T * it) \
	{ \
		if ( it ) \
		{ \
			mem_free(it->map->pool, it); \
		} \
	} \
	\
	PREFIX bool_t ITERATOR_T##_get(ITERATOR_T * it, VALUE_T * item) \
	{ \
		bool_t ok = false; \
		\
		if ( it && item ) \
		{ \
			if ( it->map_entry ) \
			{ \
				util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T)); \
				ok = true;\
			} \
		} \
		\
		return ok; \
	} \
	\
	PREFIX bool_t ITERATOR_T##_next(ITERATOR_T * it, VALUE_T * item) \
	{ \
		bool_t ok = false; \
		\
		if ( it && item ) \
		{ \
			for(uint32_t e = it->entry + 1 ; e < BUCKET_SIZE ; e++) \
			{ \
				if (it->map_bucket->entries[e]) \
				{ \
					it->entry = e; \
					it->map_entry = it->map_bucket->entries[e]; \
					ok = true; \
					util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T)); \
					break; \
				} \
			} \
			if (!ok) \
			{ \
				for ( uint32_t b = it->bucket + 1 ; b < (MAP_CAPACITY/BUCKET_SIZE) ; b++ ) \
				{ \
					if ( it->map->buckets[b] ) \
					{ \
						for (uint32_t e = 0 ; e < BUCKET_SIZE ; e++) \
						{ \
							if (it->map->buckets[b]->entries[e]) \
							{ \
								it->entry = e; \
								it->bucket = b; \
								it->map_entry = it->map->buckets[b]->entries[e]; \
								util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T)); \
								ok = true; \
								break; \
							} \
						} \
					} \
				} \
			} \
		} \
		\
		return ok; \
	} \
	\
	PREFIX void ITERATOR_T##_reset(ITERATOR_T * it) \
	{ \
		if (it) \
		{ \
			it->map_bucket = NULL; \
			it->map_entry = NULL; \
			it->entry = 0; \
			it->bucket = 0; \
			for (uint32_t b = 0 ; b < (MAP_CAPACITY/BUCKET_SIZE) ; b++ ) \
			{ \
				bool found = false; \
				if (it->map->buckets[b]) \
				{ \
					for (uint32_t e = 0 ; e < BUCKET_SIZE ; e++) \
					{ \
						if (it->map->buckets[b]->entries[e]) \
						{ \
							it->map_bucket = it->map->buckets[b]; \
							it->map_entry = it->map->buckets[b]->entries[e]; \
							it->entry = e; \
							it->bucket = b; \
							found = true; \
							break; \
						} \
					} \
				} \
				if (found) \
				{ \
					break; \
				} \
			} \
		} \
	} \
	\
	extern inline void ITERATOR_T##_test(void) \
	{ \
		ITERATOR_T * item = ITERATOR_T##_create(NULL); \
		ITERATOR_T##_next(item, NULL); \
		ITERATOR_T##_get(item, NULL); \
		ITERATOR_T##_reset(item); \
		ITERATOR_T##_delete(item); \
		ITERATOR_T##_test(); \
	} \

#endif /* HASHED_MAP_ITERATOR_H_ */
