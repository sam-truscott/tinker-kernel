/*
 *
 * TINKER Source Code
 * 
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

#include "api/tinker_api_types.h"
#include "arch/tgt_types.h"
#include "kernel/console/print_out.h"
#include "kernel/utils/hash/basic_hashes.h"
#include "kernel/utils/util_memset.h"
#include "kernel/kernel_panic.h"
#include "kernel/kernel_assert.h"

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

#if defined (DEBUG_COLLECTIONS)
#define HASH_MAP_DEBUG debug_print
#else
static inline void empty2(const char * const x, ...) {if (x){}}
#define HASH_MAP_DEBUG empty2
#endif

#define HASH_MAP_TYPE_T(HASH_MAP_T) \
	\
	typedef struct HASH_MAP_T HASH_MAP_T; \

#define HASH_MAP_INTERNAL_TYPE_T(HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY, BUCKET_SIZE) \
	\
	typedef bool_t(HASH_MAP_T##hash_key_equal)(const KEY_T l, const KEY_T r); \
	\
	typedef int32_t(HASH_MAP_T##hash_create_hash)(const void * const data, const uint32_t size);\
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
		HASH_MAP_T##_bucket_t * buckets[MAP_CAPACITY/BUCKET_SIZE]; \
		HASH_MAP_T##hash_create_hash * hashing_algorithm; \
		HASH_MAP_T##hash_key_equal * key_equal; \
		mem_pool_info_t * pool; \
		bool_t key_is_value; \
	} HASH_MAP_T##_internal_t;\
	\

#define HASH_FUNCS_VALUE(HASH_MAP_T, KEY_T) \
	\
	static inline void HASH_MAP_T##_copy_key(HASH_MAP_T##_entry_t * const e, const KEY_T r) \
	{ \
		util_memcpy(&e->key, &r, sizeof(KEY_T)); \
	} \
	\
	static int32_t HASH_MAP_T##_hash(const HASH_MAP_T * const map, const KEY_T key) \
	{ \
		/* use the assigned algorithm to generate the hash */ \
		int32_t h = map->hashing_algorithm(&key, sizeof(KEY_T)); \
		/* now modify the hash for use by this code */ \
		h ^= ((h >> 20) ^ (h >> 12)); \
        return (h ^ ((h >> 7) ^ (h >> 4))); \
	} \
	\

#define HASH_FUNCS_POINTER(HASH_MAP_T, KEY_T) \
	\
	static inline void HASH_MAP_T##_copy_key(HASH_MAP_T##_entry_t * const e, const KEY_T r) \
	{ \
		util_memcpy(e->key, r, sizeof(KEY_T)); \
	} \
	\
	static int32_t HASH_MAP_T##_hash(const HASH_MAP_T * const map, const KEY_T key) \
	{ \
		/* use the assigned algorithm to generate the hash */ \
		int32_t h = map->hashing_algorithm(key, sizeof(KEY_T)); \
		/* now modify the hash for use by this code */ \
		h ^= ((h >> 20) ^ (h >> 12)); \
        return (h ^ ((h >> 7) ^ (h >> 4))); \
	} \
	\

#define HASH_MAP_SPEC_INITALISE(PREFIX, HASH_MAP_T) \
	\
	PREFIX void HASH_MAP_T##_initialise( \
			HASH_MAP_T * const map, \
			HASH_MAP_T##hash_create_hash * const hashing_algorithm, \
			HASH_MAP_T##hash_key_equal * const hash_key_equal, \
			const bool_t key_is_value, \
			mem_pool_info_t * const pool);
#define HASH_MAP_SPEC_CREATE(PREFIX, HASH_MAP_T) \
	\
	PREFIX HASH_MAP_T * HASH_MAP_T##_create( \
			HASH_MAP_T##hash_create_hash * const hashing_algorithm, \
			HASH_MAP_T##hash_key_equal * const hash_key_equal, \
			const bool_t key_is_value, \
			mem_pool_info_t * const pool);
#define HASH_MAP_SPEC_DELETE(PREFIX, HASH_MAP_T) \
	\
	PREFIX void HASH_MAP_T##_delete(HASH_MAP_T * const map);
#define HASH_MAP_SPEC_GET(PREFIX, HASH_MAP_T, KEY_T, VALUE_T) \
	\
	PREFIX bool_t HASH_MAP_T##_get(const HASH_MAP_T * const map, const KEY_T key, VALUE_T * const value);
#define HASH_MAP_SPEC_PUT(PREFIX, HASH_MAP_T, KEY_T, VALUE_T) \
	\
	PREFIX bool_t HASH_MAP_T##_put(HASH_MAP_T * const map, const KEY_T key, VALUE_T value); \
	PREFIX int32_t HASH_MAP_T##_index_of(const HASH_MAP_T * const map, const KEY_T key);
#define HASH_MAP_SPEC_REMOVE(PREFIX, HASH_MAP_T, KEY_T, VALUE_T) \
	\
	PREFIX bool_t HASH_MAP_T##_remove(HASH_MAP_T * const map, const KEY_T key);
#define HASH_MAP_SPEC_SIZE(PREFIX, HASH_MAP_T) \
	\
	PREFIX inline uint32_t HASH_MAP_T##_size(const HASH_MAP_T * const map);
#define HASH_MAP_SPEC_CAPACITY(PREFIX, HASH_MAP_T) \
	\
	PREFIX inline uint32_t HASH_MAP_T##_capacity();
#define HASH_MAP_SPEC_CONTAINS_KEY(PREFIX, HASH_MAP_T, KEY_T) \
	\
	PREFIX bool_t HASH_MAP_T##_contains_key(const HASH_MAP_T * const map, const KEY_T key);
#define HASH_MAP_BODY_INITALISE(PREFIX, HASH_MAP_T, MAP_CAPACITY, BUCKET_SIZE) \
	\
	/*
	 * Initialise an instance that is already declared (i.e. on the stack/bss)
	 */ \
	PREFIX void HASH_MAP_T##_initialise( \
			HASH_MAP_T * const map, \
			HASH_MAP_T##hash_create_hash * const hashing_algorithm, \
			HASH_MAP_T##hash_key_equal * const hash_key_equal, \
			const bool_t key_is_value, \
			mem_pool_info_t * const pool) \
	{ \
		map->size = 0; \
		map->hashing_algorithm = hashing_algorithm; \
		map->key_equal = hash_key_equal; \
		map->pool = pool; \
		map->key_is_value = key_is_value; \
		HASH_MAP_DEBUG("hashed_map: Creating %d buckets\n", MAP_CAPACITY/BUCKET_SIZE); \
		for (uint32_t tmp = 0 ; tmp < (MAP_CAPACITY/BUCKET_SIZE) ; tmp++ ) \
		{ \
			map->buckets[tmp] = NULL; \
		} \
	}
#define HASH_MAP_BODY_CREATE(PREFIX, HASH_MAP_T) \
	/*
	 * Constructor for the map, returns a pointer to the new MAP or NULL
	 * if the allocation failed.
	 */ \
	PREFIX HASH_MAP_T * HASH_MAP_T##_create( \
			HASH_MAP_T##hash_create_hash * const hashing_algorithm, \
			HASH_MAP_T##hash_key_equal * const hash_key_equal, \
			const bool_t key_is_value, \
			mem_pool_info_t * const pool) \
	{ \
		HASH_MAP_T * const new_map = mem_alloc(pool, sizeof(HASH_MAP_T)); \
		if (new_map) \
		{ \
			HASH_MAP_T##_initialise(new_map, hashing_algorithm, hash_key_equal, key_is_value, pool); \
		} \
		return new_map; \
	}
#define HASH_MAP_BODY_DELETE(PREFIX, HASH_MAP_T, MAP_CAPACITY, BUCKET_SIZE) \
	/*
	 * Delete the hash map
	 */ \
	PREFIX void HASH_MAP_T##_delete(HASH_MAP_T * const map) \
	{ \
		if (map) \
		{ \
			for (uint32_t i = 0 ; i < MAP_CAPACITY/BUCKET_SIZE ; i++) \
			{ \
				kernel_assert("Map still has buckets", map->buckets[i]==NULL); \
			} \
		} \
		\
		if (map && map->pool) \
		{ \
			mem_free (map->pool, map); \
		} \
	}
#define HASH_MAP_BODY_GET(PREFIX, HASH_MAP_T, KEY_T, VALUE_T, BUCKET_SIZE) \
	/*
	 * Get an element from the map
	 */ \
	PREFIX bool_t HASH_MAP_T##_get(const HASH_MAP_T * const map, const KEY_T key, VALUE_T * const value) \
	{ \
	    /* find the table entry */ \
		const int32_t index = HASH_MAP_T##_index_of(map, key); \
		const HASH_MAP_T##_bucket_t * const bucket = (const HASH_MAP_T##_bucket_t *)map->buckets[index]; \
		bool_t found = false; \
		/* if found and used return the value */ \
		if (bucket && value) \
		{ \
			for (uint32_t i=0 ; i < BUCKET_SIZE && !found ; i++) \
			{ \
				if (bucket->entries[i]) \
				{ \
					if (map->key_equal(bucket->entries[i]->key,key)) \
					{ \
						HASH_MAP_DEBUG("hashed_map: get entry in bucket %d entry %d\n", index, i); \
						*value = bucket->entries[i]->value; \
						found = true; \
					} \
				} \
			} \
		} \
		return found; \
	}
#define HASH_MAP_BODY_PUT(PREFIX, HASH_MAP_T, KEY_T, VALUE_T, MAP_CAPACITY, BUCKET_SIZE) \
	/*
	 * Put an element in the map
	 */ \
	PREFIX bool_t HASH_MAP_T##_put(HASH_MAP_T * const map, const KEY_T key, VALUE_T value) \
	{ \
		bool_t put_ok = false; \
		if (map->size < MAP_CAPACITY) \
		{ \
			const int32_t index = HASH_MAP_T##_index_of(map, key); \
			HASH_MAP_T##_bucket_t * bucket; \
			if (!map->buckets[index]) \
			{ \
				map->buckets[index] = mem_alloc(map->pool, sizeof(HASH_MAP_T##_bucket_t)); \
				bucket = map->buckets[index]; \
				util_memset(bucket, 0, sizeof(HASH_MAP_T##_bucket_t)); \
			} else { \
				bucket = map->buckets[index]; \
			} \
			if (bucket) \
			{ \
				for (uint32_t i = 0 ; i < BUCKET_SIZE && !put_ok ; i++) \
				{ \
					if (!bucket->entries[i]) \
					{ \
						HASH_MAP_DEBUG("hashed_map: Putting value in bucket %d entry %d\n", index, i); \
						bucket->entries[i] = mem_alloc(map->pool, sizeof(HASH_MAP_T##_entry_t)); \
						util_memset(bucket->entries[i], 0, sizeof(HASH_MAP_T##_entry_t)); \
						HASH_MAP_T##_copy_key(bucket->entries[i], key); \
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
	 * Internal function to return the index of a key
	 */ \
	PREFIX int32_t HASH_MAP_T##_index_of(const HASH_MAP_T * const map, const KEY_T key) \
	{ \
		/*
		 * normalise the hashed value to be a suitable index within
		 * the range of the tables index
		 */ \
		return HASH_MAP_T##_hash(map, key) & ((MAP_CAPACITY/BUCKET_SIZE) - 1); \
	}
#define HASH_MAP_BODY_REMOVE(PREFIX, HASH_MAP_T, KEY_T, BUCKET_SIZE) \
	/*
	 * Remove an element from the map
	 */ \
	PREFIX bool_t HASH_MAP_T##_remove(HASH_MAP_T * const map, const KEY_T key) \
	{ \
		 bool_t ok = false; \
		 const int32_t index = HASH_MAP_T##_index_of(map, key); \
		 HASH_MAP_T##_bucket_t * const bucket = map->buckets[index]; \
		 if (bucket) \
		 { \
			 uint8_t c = 0; \
			 for (uint32_t i = 0 ; i < BUCKET_SIZE; i++) \
			 { \
				 if (bucket->entries[i]) \
				 { \
					 c++; \
					 if (!ok && map->key_equal(bucket->entries[i]->key,key)) \
					 { \
						 HASH_MAP_DEBUG("hashed_map: removing entry in bucket %d entry %d\n", index, i); \
						 ok = true; \
						 map->size--; \
						 mem_free(map->pool, bucket->entries[i]); \
						 bucket->entries[i] = NULL; \
						 c--; \
					 } \
				 } \
			 } \
			 if (!c) \
			 { \
				 HASH_MAP_DEBUG("hashed_map: bucket %d is empty, removing it\n", index); \
				 mem_free(map->pool, bucket); \
				 map->buckets[index] = NULL; \
			 } \
			 else \
			 { \
				 HASH_MAP_DEBUG("hashed_map: bucket %d has %d elements left\n", index, c); \
			 } \
		 } \
		 \
		 return ok; \
	}
#define HASH_MAP_BODY_SIZE(PREFIX, HASH_MAP_T) \
	/*
	 * Get the number of elements in the map
	 */ \
	PREFIX inline uint32_t HASH_MAP_T##_size(const HASH_MAP_T * const map) \
	{ \
		return map->size; \
	}
#define HASH_MAP_BODY_CAPACITY(PREFIX, HASH_MAP_T, MAP_CAPACITY) \
	/*
	 * Get the total capacity of the map
	 */ \
	PREFIX inline uint32_t HASH_MAP_T##_capacity() \
	{ \
		 return MAP_CAPACITY; \
	}
#define HASH_MAP_BODY_CONTAINS_KEY(PREFIX, HASH_MAP_T, KEY_T, BUCKET_SIZE) \
	/*
	 * Does the map contain a given key?
	 */ \
	PREFIX bool_t HASH_MAP_T##_contains_key(const HASH_MAP_T * const map, const KEY_T key) \
	{ \
		bool_t found = false; \
		if (map) \
		{ \
			const int32_t index = HASH_MAP_T##_index_of(map, key); \
			HASH_MAP_DEBUG("hashed_map: checking for key in bucket %d\n", index); \
			if (map->buckets[index]) \
			{ \
				const HASH_MAP_T##_bucket_t * const bucket = map->buckets[index]; \
				for (uint32_t i = 0 ; i < BUCKET_SIZE && !found ; i++) \
				{ \
					if (bucket->entries[i]) \
					{ \
						if (map->key_equal(bucket->entries[i]->key,key)) \
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

#endif /* HASHED_MAP_H_ */
