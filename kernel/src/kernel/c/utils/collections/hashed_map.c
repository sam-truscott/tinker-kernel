/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "utils/collections/hashed_map.h"

#if defined (DEBUG_COLLECTIONS)
#define HASH_MAP_DEBUG debug_print
#else
static inline void empty2(const char * const x, ...) {if (x){}}
#define HASH_MAP_DEBUG empty2
#endif

typedef struct map_entry
{
	uint32_t key;
	void * value;
} map_entry_t;

typedef struct map_bucket
{
	map_entry_t * entries[BUCKET_SIZE];
} map_bucket_t;

typedef struct map_t
{
	uint32_t size;
	uint32_t key_size;
	uint32_t capacity;
	uint32_t bucket_size;
	map_bucket_t * buckets[MAP_CAPACITY/BUCKET_SIZE];
	map_create_hash * hashing_algorithm;
	mem_pool_info_t * pool;
} map_internal_t;

typedef struct map_it_t
{
	map_t * map;
	map_entry_t * map_entry;
	map_bucket_t * map_bucket;
	uint32_t bucket;
	uint32_t entry;
} map_it_internal_t;

static int32_t map_hash(const map_t * const map, const void * key)
{
	/* use the assigned algorithm to generate the hash */
	int32_t h = map->hashing_algorithm(key, map->key_size);
	/* now modify the hash for use by this code */
	h ^= ((h >> 20) ^ (h >> 12));
	return (h ^ ((h >> 7) ^ (h >> 4)));
}

/*
 * Initialise an instance that is already declared (i.e. on the stack/bss)
 */
void map_initialise(
		map_t * const map,
		map_create_hash * const hashing_algorithm,
		mem_pool_info_t * const pool,
		uint32_t capacity,
		uint32_t bucket_size)
{
	map->capacity = capacity;
	map->bucket_size = bucket_size;
	map->size = 0;
	map->hashing_algorithm = hashing_algorithm;
	map->key_equal = hash_key_equal;
	map->pool = pool;
	map->key_is_value = key_is_value;
	HASH_MAP_DEBUG("hashed_map: Creating %d buckets\n", capacity/bucket_size);
	for (uint32_t tmp = 0 ; tmp < (capacity/bucket_size) ; tmp++ )
	{
		map->buckets[tmp] = NULL;
	}
}

/*
 * Constructor for the map, returns a pointer to the new MAP or NULL
 * if the allocation failed.
 */
map_t * map_create(
		map_create_hash * const hashing_algorithm,
		mem_pool_info_t * const pool,
		uint32_t capacity,
		uint32_t bucket_size)
{
	map_t * const new_map = mem_alloc(pool, sizeof(map_t));
	if (new_map)
	{
		map_initialise(new_map, hashing_algorithm, pool, capacity, bucket_size);
	}
	return new_map;
}

/*
 * Delete the hash map
 */
void map_delete(map_t * const map)
{
	if (map)
	{
		for (uint32_t i = 0 ; i < map->capacity/map->bucket_size ; i++)
		{
			kernel_assert("Map still has buckets", map->buckets[i]==NULL);
		}
	}

	if (map && map->pool)
	{
		mem_free (map->pool, map);
	}
}

/*
 * Get an element from the map
 */
bool_t map_get(const map_t * const map, const void * key, void ** const value)
{
	/* find the table entry */
	const int32_t index = HASH_MAP_T##_index_of(map, key);
	const HASH_MAP_T##_bucket_t * const bucket = (const HASH_MAP_T##_bucket_t *)map->buckets[index];
	bool_t found = false;
	/* if found and used return the value */
	if (bucket && value)
	{
		for (uint32_t i=0 ; i < BUCKET_SIZE && !found ; i++)
		{
			if (bucket->entries[i])
			{
				if (map->key_equal(bucket->entries[i]->key,key))
				{
					HASH_MAP_DEBUG("hashed_map: get entry in bucket %d entry %d\n", index, i);
					*value = bucket->entries[i]->value;
					found = true;
				}
			}
		}
	}
	return found;
}

/*
 * Put an element in the map
 */
bool_t map_put(map_t * const map, const void * key, void * value)
{
	bool_t put_ok = false;
	if (map->size < map->capacity)
	{
		const int32_t index = HASH_MAP_T##_index_of(map, key);
		HASH_MAP_T##_bucket_t * bucket;
		if (!map->buckets[index])
		{
			map->buckets[index] = mem_alloc(map->pool, sizeof(HASH_MAP_T##_bucket_t));
			bucket = map->buckets[index];
			util_memset(bucket, 0, sizeof(HASH_MAP_T##_bucket_t));
		}
		else
		{
			bucket = map->buckets[index];
		}
		if (bucket)
		{
			int32_t used_bucket_index = -1;
			int32_t unused_bucket_index = -1;
			for (uint32_t i = 0 ; i < BUCKET_SIZE && used_bucket_index == -1 ; i++)
			{
				if (bucket->entries[i])
				{
					if (map->key_equal(bucket->entries[i]->key,key))
					{
						used_bucket_index = i;
					}
				}
				else
				{
					unused_bucket_index = i;
				}
			}
			int32_t bucket_index;
			if (used_bucket_index != -1)
			{
				bucket_index = used_bucket_index;
			}
			else
			{
				bucket_index = unused_bucket_index;
			}
			if (bucket_index != -1)
			{
				HASH_MAP_DEBUG("hashed_map: Putting value in bucket %d entry %d\n", index, bucket_index);
				bucket->entries[bucket_index] = mem_alloc(map->pool, sizeof(HASH_MAP_T##_entry_t));
				util_memset(bucket->entries[bucket_index], 0, sizeof(HASH_MAP_T##_entry_t));
				HASH_MAP_T##_copy_key(bucket->entries[bucket_index], key);
				bucket->entries[bucket_index]->value = value;
				put_ok = true;
				map->size++;
			}
		}
	}
	return put_ok;
}

/*
 * Internal function to return the index of a key
 */
static int32_t map_index_of(const map_t * const map, const void * key)
{
	/*
	 * normalise the hashed value to be a suitable index within
	 * the range of the tables index
	 */
	return_hash(map, key) & ((MAP_CAPACITY/BUCKET_SIZE) - 1);
}

/*
 * Remove an element from the map
 */
bool_t map_remove(map_t * const map, const void * key)
{
	 bool_t ok = false;
	 const int32_t index = HASH_MAP_T##_index_of(map, key);
	 map_bucket_t * const bucket = map->buckets[index];
	 if (bucket)
	 {
		 uint8_t c = 0;
		 for (uint32_t i = 0 ; i < BUCKET_SIZE; i++)
		 {
			 if (bucket->entries[i])
			 {
				 c++;
				 if (!ok && map->key_equal(bucket->entries[i]->key,key))
				 {
					 HASH_MAP_DEBUG("hashed_map: removing entry in bucket %d entry %d\n", index, i);
					 ok = true;
					 map->size--;
					 mem_free(map->pool, bucket->entries[i]);
					 bucket->entries[i] = NULL;
					 c--;
				 }
			 }
		 }
		 if (!c)
		 {
			 HASH_MAP_DEBUG("hashed_map: bucket %d is empty, removing it\n", index);
			 mem_free(map->pool, bucket);
			 map->buckets[index] = NULL;
		 }
		 else
		 {
			 HASH_MAP_DEBUG("hashed_map: bucket %d has %d elements left\n", index, c);
		 }
	 }

	 return ok;
}

/*
 * Get the number of elements in the map
 */
uint32_t map_size(const map_t * const map)
{
	return map->size;
}

/*
 * Get the total capacity of the map
 */
uint32_t map_capacity(const map_t * const map)
{
	 return map->capacity;
}

/*
 * Does the map contain a given key?
 */
bool_t map_contains_key(const map_t * const map, const void * key)
{
	bool_t found = false;
	if (map)
	{
		const int32_t index = HASH_MAP_T##_index_of(map, key);
		HASH_MAP_DEBUG("hashed_map: checking for key in bucket %d\n", index);
		if (map->buckets[index])
		{
			const map_bucket_t * const bucket = map->buckets[index];
			for (uint32_t i = 0 ; i < BUCKET_SIZE && !found ; i++)
			{
				if (bucket->entries[i])
				{
					if (map->key_equal(bucket->entries[i]->key,key))
					{
						found = true;
					}
				}
			}
		}
	}

	return found;
}

void map_it_t_initialise(map_it_t * it, map_t * const map)
{
	if (it && map)
	{
		it->map = map;
		map_it_reset(it);
	}
}

map_it_t * map_it_create(map_t * const map)
{
	map_it_t * it = NULL;
	if (map)
	{
		it = mem_alloc(map->pool, sizeof(map_it_t));
		map_it_initialise(it, map);
	}
	return it;
}

void map_it_delete(map_it_t * it)
{
	if (it && it->map && it->map->pool)
	{
		mem_free(it->map->pool, it);
	}
}

bool_t map_it_get(map_it_t * const it, void ** item)
{
	bool_t ok = false;

	if (it && item)
	{
		if (it->map_entry)
		{
			util_memcpy(item, &(it->map_entry->value), sizeof(void*));
			ok = true;
		}
	}

	return ok;
}

bool_t map_it_next(map_it_t * it, void ** item)
{
	bool_t ok = false;

	if ( it && item )
	{
		for(uint32_t e = it->entry + 1 ; e < it->map->bucket_size ; e++)
		{
			if (it->map_bucket->entries[e])
			{
				it->entry = e;
				it->map_entry = it->map_bucket->entries[e];
				ok = true;
				util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T));
				break;
			}
		}
		if (!ok)
		{
			for ( uint32_t b = it->bucket + 1 ; b < (MAP_CAPACITY/BUCKET_SIZE) && !ok ; b++ )
			{
				if ( it->map->buckets[b] )
				{
					for (uint32_t e = 0 ; e < BUCKET_SIZE ; e++)
					{
						if (it->map->buckets[b]->entries[e])
						{
							it->entry = e;
							it->bucket = b;
							it->map_entry = it->map->buckets[b]->entries[e];
							util_memcpy(item, &(it->map_entry->value), sizeof(VALUE_T));
							ok = true;
							break;
						}
					}
				}
			}
		}
	}

	return ok;
}

void map_it_reset(map_it_t * it)
{
	if (it)
	{
		it->map_bucket = NULL;
		it->map_entry = NULL;
		it->entry = 0;
		it->bucket = 0;
		for (uint32_t b = 0 ; b < (MAP_CAPACITY/BUCKET_SIZE) ; b++ )
		{
			bool found = false;
			if (it->map->buckets[b])
			{
				for (uint32_t e = 0 ; e < BUCKET_SIZE && !found; e++)
				{
					if (it->map->buckets[b]->entries[e])
					{
						it->map_bucket = it->map->buckets[b];
						it->map_entry = it->map->buckets[b]->entries[e];
						it->entry = e;
						it->bucket = b;
						found = true;
						break;
					}
				}
			}
			if (found)
			{
				break;
			}
		}
	}
}
