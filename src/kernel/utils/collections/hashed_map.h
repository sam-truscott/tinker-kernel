/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
/*
 * ##########################################
 * ##########################################
 * ##
 * ## The algorithms for the hashing - HASH_MAP_T##_hash - is taken
 * ## from the HashMap class of Java JDK 6 and is used here to get a
 * ## correct implementation together to generated suitable test-cases.
 * ##
 * ## Once these have been created the implementation should be changed
 * ## so it doesn't contain any JDK implementation details
 * ##
 * ##########################################
 * ##########################################
 */

#ifndef HASHED_MAP_H_
#define HASHED_MAP_H_

#include "api/sos_api_types.h"
#include "../hash/basic_hashes.h"
#include "../util_memset.h"

/*
 * This abstract data type is used to model a hash-map.
 *
 * The KEY_T can be of most types but ideally should be discrete to
 * enable the contains_key function to work. The contains_value has
 * reduced in functionality because of this as it'll simply compare
 * the address of the values, not the actual value, because if it's
 * a complex type (struct) then the code will fall over.
 *
 * MAP_CAPACITY must be a power of 2 but this isn't enforced
 * by this code (yet). This is for the hashing algorithms
 * (technically it's the indexing part to ensure it's in range).
 *
 * First an abstract data type must be created or, if declared statically,
 * initialised using _create or _initialise.
 *
 * Then the user is free to put, get and remove entries.
 *
 * The details of the hash-map can be queried. The 'capacity' is the
 * total size of the table and the 'size' is the total number of
 * elements in the map.
 *
 * Once finished, if creating using _create, the _delete procedure
 * should be called to return all the user memory back to the heap.
 *
 * The hash-map is *NOT* synchronised and is therefore not thread safe.
 *
 */
#define BUCKET_SIZE 10
#define HASH_MAP_TYPE_T(HASH_MAP_T) \
	\
	typedef struct HASH_MAP_T HASH_MAP_T; \

#define HASH_MAP_INTERNAL_TYPE_T(HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY) \
	\
	typedef struct HASH_MAP_T##_entry \
	{ \
		KEY_T key; \
		VALUE_T value; \
	} HASH_MAP_T##_entry_t; \
	\
	typedef struct HASH_MAP_T##_bucket \
	{ \
		HASH_MAP_T##_entry_t * entries[BUCKET_SIZE]; \
	} HASH_MAP_T##_bucket_t; \
	\
	typedef struct HASH_MAP_T \
	{ \
		uint32_t size; \
		uint32_t capacity; \
		HASH_MAP_T##_bucket_t * buckets[MAP_CAPACITY/BUCKET_SIZE]; \
		__hash_create_hash * hashing_algorithm; \
		__mem_pool_info_t * pool; \
	} HASH_MAP_T##_internal_t;\
	\

#define HASH_MAP_SPEC_T(PREFIX, HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY) \
	\
	PREFIX void HASH_MAP_T##_initialise( \
			HASH_MAP_T * map, \
			__hash_create_hash * hashing_algorithm, \
			__mem_pool_info_t * pool); \
	\
	PREFIX HASH_MAP_T * HASH_MAP_T##_create( \
			__hash_create_hash * hashing_algorithm, \
			__mem_pool_info_t * pool); \
	\
	PREFIX void HASH_MAP_T##_delete(HASH_MAP_T * map); \
	\
	PREFIX bool HASH_MAP_T##_get(const HASH_MAP_T * map, const KEY_T key, VALUE_T * value); \
	\
	PREFIX bool HASH_MAP_T##_put(HASH_MAP_T * map, KEY_T key, VALUE_T value); \
	\
	PREFIX bool HASH_MAP_T##_remove(HASH_MAP_T * map, const KEY_T key); \
	\
	PREFIX uint32_t HASH_MAP_T##_size(const HASH_MAP_T * map); \
	\
	PREFIX uint32_t HASH_MAP_T##_capacity(const HASH_MAP_T * map); \
	\
	PREFIX bool HASH_MAP_T##_contains_key(const HASH_MAP_T * map, const KEY_T key); \
	\

#define HASH_MAP_BODY_T(PREFIX, HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY) \
	\
	/*
	 * Initialise an instance that is already declared (i.e. on the stack/bss)
	 */ \
	PREFIX void HASH_MAP_T##_initialise( \
			HASH_MAP_T * map, \
			__hash_create_hash * hashing_algorithm, \
			__mem_pool_info_t * pool) \
	{ \
		map->size = 0; \
		map->capacity = MAP_CAPACITY; \
		map->hashing_algorithm = hashing_algorithm; \
		map->pool = pool; \
		for ( uint32_t tmp = 0 ; tmp < MAP_CAPACITY/BUCKET_SIZE ; tmp++ ) \
		{ \
			map->buckets[tmp] = NULL; \
		} \
	} \
	\
	/*
	 * Constructor for the map, returns a pointer to the new MAP or NULL
	 * if the allocation failed.
	 */ \
	PREFIX HASH_MAP_T * HASH_MAP_T##_create( \
			__hash_create_hash * hashing_algorithm, \
			__mem_pool_info_t * pool) \
	{ \
		HASH_MAP_T * new_map = NULL; \
		\
		/* allocate space */ \
		new_map = __mem_alloc(pool, sizeof(HASH_MAP_T)); \
		/* initialise the data */ \
		HASH_MAP_T##_initialise(new_map, hashing_algorithm, pool); \
		return new_map; \
	} \
	\
	/*
	 * Delete the hash map
	 */ \
	PREFIX void HASH_MAP_T##_delete(HASH_MAP_T * map) \
	{ \
		 if ( map != NULL && map->pool != NULL ) \
		 { \
			__mem_free (map->pool, map); \
		 } \
	} \
	\
	/*
	 * Internal function to generate the hash
	 */ \
	PREFIX inline int32_t HASH_MAP_T##_hash(const HASH_MAP_T * map, const KEY_T key) \
	{ \
		/* use the assigned algorithm to generate the hash */ \
		int32_t h = map->hashing_algorithm(&key, sizeof(KEY_T)); \
		/* now modify the hash for use by this code */ \
		h ^= ((h >> 20) ^ (h >> 12)); \
        return (h ^ ((h >> 7) ^ (h >> 4))); \
	} \
	\
	/*
	 * Internal function to return the index of a key
	 */ \
	PREFIX inline int32_t HASH_MAP_T##_index_of(const HASH_MAP_T * map, const KEY_T key) \
	{ \
		/*
		 * normalise the hashed value to be a suitable index within
		 * the range of the tables index
		 */ \
		return HASH_MAP_T##_hash(map, key) & ((map->capacity/BUCKET_SIZE) - 1); \
	} \
	\
	/*
	 * Get an element from the map
	 */ \
	PREFIX bool HASH_MAP_T##_get(const HASH_MAP_T * map, const KEY_T key, VALUE_T * value) \
	{ \
		const HASH_MAP_T##_bucket_t * bucket; \
		bool found = false; \
		/* find the table entry */ \
		bucket = (const HASH_MAP_T##_bucket_t *) \
			map->buckets[HASH_MAP_T##_index_of(map, key)]; \
		/* if found and used return the value */ \
		if ( bucket && value != NULL) \
		{ \
			uint32_t i; \
			for (i=0 ; i < BUCKET_SIZE && !found ; i++) \
			{ \
				if (bucket->entries[i]) \
				{ \
					if (bucket->entries[i]->key == key) \
					{ \
						*value = bucket->entries[i]->value; \
						found = true; \
					} \
				} \
			} \
		} \
		return found; \
	} \
	\
	/*
	 * Put an element in the map
	 */ \
	PREFIX bool HASH_MAP_T##_put(HASH_MAP_T * map, KEY_T key, VALUE_T value) \
	{ \
		bool put_ok = false; \
		if ( map->size < map->capacity ) \
		{ \
			const int32_t index = HASH_MAP_T##_index_of(map, key); \
			if (!map->buckets[index]) \
			{ \
				map->buckets[index] = \
					__mem_alloc(map->pool, sizeof(HASH_MAP_T##_bucket_t)); \
			} \
			HASH_MAP_T##_bucket_t * bucket = map->buckets[index]; \
			if (bucket) \
			{ \
				uint32_t i; \
				for (i = 0 ; i < BUCKET_SIZE && !put_ok ; i++) \
				{ \
					if (!bucket->entries[i]) \
					{ \
						bucket->entries[i] = __mem_alloc(map->pool, sizeof(HASH_MAP_T##_entry_t)); \
						bucket->entries[i]->key = key; \
						bucket->entries[i]->value = value; \
						put_ok = true; \
						map->size++; \
					} \
				} \
			} \
		} \
		return put_ok; \
	} \
	\
	/*
	 * Remove an element from the map
	 */ \
	PREFIX bool HASH_MAP_T##_remove(HASH_MAP_T * map, const KEY_T key) \
	{ \
		 bool ok = false; \
		 HASH_MAP_T##_bucket_t * bucket = NULL; \
		 \
		 bucket = map->buckets[HASH_MAP_T##_index_of(map, key)]; \
		 if ( bucket ) \
		 { \
			 uint32_t i; \
			 for (i = 0 ; i < BUCKET_SIZE && !ok; i++) \
			 { \
				 if (bucket->entries[i]->key == key) \
				 { \
					 ok = true; \
					 map->size--; \
					 bucket->entries[i] = NULL; \
					 __mem_free(map->pool, bucket->entries[i]); \
				 } \
			 } \
		 } \
		 \
		 return ok; \
	} \
	/*
	 * Get the number of elements in the map
	 */ \
	PREFIX inline uint32_t HASH_MAP_T##_size(const HASH_MAP_T * map) \
	{ \
		return map->size; \
	} \
	/*
	 * Get the total capacity of the map
	 */ \
	PREFIX inline uint32_t HASH_MAP_T##_capacity(const HASH_MAP_T * map) \
	{ \
		 return map->capacity; \
	} \
	\
	/*
	 * Does the map contain a given key?
	 */ \
	PREFIX bool HASH_MAP_T##_contains_key(const HASH_MAP_T * map, const KEY_T key) \
	{ \
		uint32_t tmp = 0; \
		bool found = false; \
		for ( ; tmp < (map->capacity/BUCKET_SIZE) && !found ; tmp++ ) \
		{ \
			const HASH_MAP_T##_bucket_t * bucket = map->buckets[tmp]; \
			if (bucket) \
			{ \
				uint32_t i; \
				for (i = 0 ; i < BUCKET_SIZE && !found ; i++) \
				{ \
					if (bucket->entries[i]) \
					{ \
						if (bucket->entries[i]->key == key) \
						{ \
							found = true; \
						} \
					} \
				} \
			} \
		} \
		\
		return found; \
	} \
	\
	extern inline void HASH_MAP_T##_test__(void) \
	{ \
		HASH_MAP_T * map = HASH_MAP_T##_create(__hash_basic_integer, NULL); \
		HASH_MAP_T##_get(map, (KEY_T)0, NULL); \
		HASH_MAP_T##_contains_key(map, (KEY_T)0); \
		HASH_MAP_T##_remove(map, (KEY_T)0); \
		HASH_MAP_T##_capacity(map); \
		HASH_MAP_T##_size(map); \
		HASH_MAP_T##_delete(map); \
	} \

#endif /* HASHED_MAP_H_ */
