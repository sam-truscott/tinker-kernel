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

#include "tinker_api_types.h"
#include "tgt_types.h"
#include "console/print_out.h"
#include "utils/hash/basic_hashes.h"
#include "utils/util_memset.h"
#include "kernel_panic.h"
#include "kernel_assert.h"

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

typedef struct map_t map_t;
typedef struct map_it_t map_it_t;

typedef int32_t (map_create_hash)(const void * const data, const uint32_t size);

void map_initialise(
		map_t * const map,
		map_create_hash * const hashing_algorithm,
		mem_pool_info_t * const pool,
		uint32_t capacity,
		uint32_t bucket_size);

map_t * map_create(
		map_create_hash * const hashing_algorithm,
		mem_pool_info_t * const pool,
		uint32_t capacity,
		uint32_t bucket_size);

void map_delete(map_t * const map);
bool_t map_get(const map_t * const map, const void * key, void ** const value);
bool_t map_put(map_t * const map, const void * key, void * value);
bool_t map_remove(map_t * const map, const void * key);
uint32_t map_size(const map_t * const map);
uint32_t map_capacity(const map_t * const map);
bool_t map_contains_key(const map_t * const map, const void * key);

#endif /* HASHED_MAP_H_ */
