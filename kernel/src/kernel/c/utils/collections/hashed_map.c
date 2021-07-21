/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "utils/collections/hashed_map.h"

#include "console/print_out.h"
#include "utils/hash/basic_hashes.h"
#include "utils/util_memset.h"
#include "utils/util_memcpy.h"
#include "kernel_panic.h"
#include "kernel_assert.h"

#define MAP_BUCKET_SIZE 32
#define MAP_CAPACITY 65535
#define MAP_BUCKET_COUNT (MAP_CAPACITY/MAP_BUCKET_SIZE)
#define MAP_MAX_KEY_LENGTH 64

#if defined (DEBUG_COLLECTIONS)
#define HASH_MAP_DEBUG debug_print
#else
static inline void empty2(const char * const x, ...) {if (x){}}
#define HASH_MAP_DEBUG empty2
#endif

typedef struct map_entry
{
	uint8_t key[MAP_MAX_KEY_LENGTH];
	void * value;
} map_entry_t;

typedef struct map_bucket
{
	map_entry_t * entries[MAP_BUCKET_SIZE];
} map_bucket_t;

typedef struct map_t
{
	uint32_t size;
	uint32_t key_size;
	map_bucket_t * buckets[MAP_BUCKET_COUNT];
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

static int32_t map_hash(const map_t * const map, const void * key);

static bool_t map_key_equals(
		const void * const key1,
		const void * const key2,
		const uint32_t key_size);

static void map_copy_key(
		const void * const key1,
		const void * const key2,
		const uint32_t key_size);

static int32_t map_index_of(const map_t * const map, const void * key);

static int32_t map_hash(const map_t * const map, const void * key)
{
	/* use the assigned algorithm to generate the hash */
	int32_t h = map->hashing_algorithm(key, map->key_size);
	/* now modify the hash for use by this code */
	h ^= ((h >> 20) ^ (h >> 12));
	return (h ^ ((h >> 7) ^ (h >> 4)));
}

static void map_copy_key(
		const void * const dst,
		const void * const src,
		const uint32_t key_size)
{
	uint8_t * left = (uint8_t*)dst;
	const uint8_t * right = (uint8_t*)src;
	for (uint32_t i = 0 ; i < key_size ; i++)
	{
		*left = *right;
		left++;
		right++;
	}
}

static bool_t map_key_equals(
		const void * const key1,
		const void * const key2,
		const uint32_t key_size)
{
	uint8_t * left = (uint8_t*)key1;
	uint8_t * right = (uint8_t*)key2;
	for (uint32_t i = 0 ; i < key_size ; i++)
	{
		if (*left != *right)
		{
			return false;
		}
		left++;
		right++;
	}
	return true;
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
	return map_hash(map, key) & (MAP_BUCKET_COUNT - 1);
}

/*
 * Initialise an instance that is already declared (i.e. on the stack/bss)
 */
void map_initialise(
		map_t * const map,
		map_create_hash * const hashing_algorithm,
		mem_pool_info_t * const pool,
		uint32_t key_size)
{
	util_memset(map, 0, sizeof(map_t));
	map->size = 0;
	map->hashing_algorithm = hashing_algorithm;
	map->pool = pool;
	map->key_size = key_size;
	kernel_assert("Map still has buckets", key_size <= MAP_MAX_KEY_LENGTH);
	HASH_MAP_DEBUG("hashed_map: Creating %d buckets\n", MAP_BUCKET_COUNT);
}

/*
 * Constructor for the map, returns a pointer to the new MAP or NULL
 * if the allocation failed.
 */
map_t * map_create(
		map_create_hash * const hashing_algorithm,
		mem_pool_info_t * const pool,
		uint32_t key_size)
{
	map_t * const new_map = mem_alloc(pool, sizeof(map_t));
	if (new_map)
	{
		map_initialise(new_map, hashing_algorithm, pool, key_size);
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
		for (uint32_t i = 0 ; i < MAP_BUCKET_COUNT ; i++)
		{
			kernel_assert("Map still has buckets", map->buckets[i] == NULL);
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
	const int32_t index = map_index_of(map, key);
	const map_bucket_t * const bucket = (const map_bucket_t *)map->buckets[index];
	bool_t found = false;
	/* if found and used return the value */
	if (bucket && value)
	{
		for (uint32_t i=0 ; i < MAP_BUCKET_SIZE && !found ; i++)
		{
			if (bucket->entries[i])
			{
				if (map_key_equals(bucket->entries[i]->key, key, map->key_size))
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
	if (map->size < MAP_CAPACITY)
	{
		const int32_t index = map_index_of(map, key);
		map_bucket_t * bucket;
		if (!map->buckets[index])
		{
			map->buckets[index] = mem_alloc(map->pool, sizeof(map_bucket_t));
			if (map->buckets[index])
			{
				bucket = map->buckets[index];
				util_memset(bucket, 0, sizeof(map_bucket_t));
			}
		}
		else
		{
			bucket = map->buckets[index];
		}
		if (bucket)
		{
			int32_t used_bucket_index = -1;
			int32_t unused_bucket_index = -1;
			for (uint32_t i = 0 ; i < MAP_BUCKET_SIZE && used_bucket_index == -1 ; i++)
			{
				if (bucket->entries[i])
				{
					if (map_key_equals(bucket->entries[i]->key, key, map->key_size))
					{
						used_bucket_index = i;
					}
				}
				else if (unused_bucket_index == -1)
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
				bucket->entries[bucket_index] = mem_alloc(map->pool, sizeof(map_entry_t));
				util_memset(bucket->entries[bucket_index], 0, sizeof(map_entry_t));
				map_copy_key(bucket->entries[bucket_index], key, map->key_size);
				bucket->entries[bucket_index]->value = value;
				put_ok = true;
				map->size++;
			}
		}
	}
	return put_ok;
}

/*
 * Remove an element from the map
 */
bool_t map_remove(map_t * const map, const void * key)
{
	 bool_t ok = false;
	 const int32_t index = map_index_of(map, key);
	 map_bucket_t * const bucket = map->buckets[index];
	 if (bucket)
	 {
		 uint8_t c = 0;
		 for (uint32_t i = 0 ; i < MAP_BUCKET_SIZE; i++)
		 {
			 if (bucket->entries[i])
			 {
				 c++;
				 if (!ok && map_key_equals(bucket->entries[i]->key, key, map->key_size))
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
	(void)map; // UNUSED
	 return MAP_CAPACITY;
}

/*
 * Does the map contain a given key?
 */
bool_t map_contains_key(const map_t * const map, const void * key)
{
	bool_t found = false;
	if (map)
	{
		const int32_t index = map_index_of(map, key);
		HASH_MAP_DEBUG("hashed_map: checking for key in bucket %d\n", index);
		if (map->buckets[index])
		{
			const map_bucket_t * const bucket = map->buckets[index];
			for (uint32_t i = 0 ; i < MAP_BUCKET_SIZE && !found ; i++)
			{
				if (bucket->entries[i])
				{
					if (map_key_equals(bucket->entries[i]->key, key, map->key_size))
					{
						found = true;
					}
				}
			}
		}
	}

	return found;
}

void map_it_initialise(map_it_t * const it, map_t * const map)
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
	if (map && map->pool)
	{
		it = mem_alloc(map->pool, sizeof(map_it_t));
		map_it_initialise(it, map);
	}
	return it;
}

void map_it_delete(map_it_t * const it)
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

bool_t map_it_next(map_it_t * const it, void ** item)
{
	bool_t ok = false;

	if ( it && item )
	{
		for(uint32_t e = it->entry + 1 ; e < MAP_BUCKET_SIZE ; e++)
		{
			if (it->map_bucket->entries[e])
			{
				it->entry = e;
				it->map_entry = it->map_bucket->entries[e];
				ok = true;
				*item = it->map_entry->value;
				break;
			}
		}
		if (!ok)
		{
			for ( uint32_t b = it->bucket + 1 ; b < MAP_BUCKET_COUNT && !ok ; b++ )
			{
				if ( it->map->buckets[b] )
				{
					for (uint32_t e = 0 ; e < MAP_BUCKET_SIZE ; e++)
					{
						if (it->map->buckets[b]->entries[e])
						{
							it->entry = e;
							it->bucket = b;
							it->map_entry = it->map->buckets[b]->entries[e];
							*item = it->map_entry->value;
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

void map_it_reset(map_it_t * const it)
{
	if (it)
	{
		it->map_bucket = NULL;
		it->map_entry = NULL;
		it->entry = 0;
		it->bucket = 0;
		for (uint32_t b = 0 ; b < MAP_BUCKET_COUNT ; b++ )
		{
			bool found = false;
			if (it->map->buckets[b])
			{
				for (uint32_t e = 0 ; e < MAP_BUCKET_SIZE && !found; e++)
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
