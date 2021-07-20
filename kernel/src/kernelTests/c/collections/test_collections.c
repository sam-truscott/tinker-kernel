/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "kernel_assert.h"
#include "kernel_panic.h"
#include "memory/memory_manager.h"
#include "utils/collections/unbounded_list.h"
#include "utils/collections/hashed_map.h"
#include "utils/hash/basic_hashes.h"
#include "utils/collections/stack.h"
#include "utils/collections/unbounded_queue.h"
#include "console/print_out.h"

#define UNIT_TESTS 1

#if defined(UNIT_TESTS)

#define TEST_POOL_SIZE 256 * 1024
#define MAP_BUCKET_SIZE 16

static void test_list(mem_pool_info_t * const pool)
{
	list_t * list = list_create(pool);

	// add and remove
	kernel_assert("size should be 0", list_size(list) == 0);
	kernel_assert("should return true", list_add(list, (void*)0));
	kernel_assert("size should be 1", list_size(list) == 1);
	uint32_t value = 1;
	kernel_assert("should return true", list_get(list, 0, (void*)&value));
	kernel_assert("size should be 0", value == 0);
	kernel_assert("should return true", list_remove(list, 0));
	kernel_assert("size should be 0", list_size(list) == 0);

	// add and remove
	kernel_assert("should return true", list_add(list, (void*)1));
	kernel_assert("should return true", list_add(list, (void*)2));
	kernel_assert("should return true", list_add(list, (void*)3));
	kernel_assert("size should be 3", list_size(list) == 3);
	kernel_assert("should return true", list_get(list, 0, (void*)&value));
	kernel_assert("size should be 3", list_size(list) == 3);
	kernel_assert("should return true", list_remove(list, 0));
	kernel_assert("should return true", list_get(list, 0, (void*)&value));
	kernel_assert("size should be 2", list_size(list) == 2);
	kernel_assert("should return true", list_remove(list, 0));
	kernel_assert("should return true", list_get(list, 0, (void*)&value));
	kernel_assert("should return true", list_remove(list, 0));
	kernel_assert("size should be 0", list_size(list) == 0);

	kernel_assert("should return true", list_add(list, (void*)1));
	kernel_assert("size should be 0", list_size(list) == 1);
	kernel_assert("should return true", list_remove_item(list, (void*)1));
	kernel_assert("size should be 0", list_size(list) == 0);

	kernel_assert("should return true", list_insert(list, 0, (void*)1));
	kernel_assert("should return true", list_insert(list, 1, (void*)2));
	kernel_assert("should return true", list_insert(list, 2, (void*)3));
	kernel_assert("size should be 3", list_size(list) == 3);
	kernel_assert("should return false", !list_insert(list, 6, (void*)6));
	kernel_assert("size should be 3", list_size(list) == 3);

	kernel_assert("should return true", list_head_to_tail(list));
	kernel_assert("size should be 3", list_size(list) == 3);
	kernel_assert("should return true", list_get(list, 0, (void*)&value));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("should return true", list_get(list, 1, (void*)&value));
	kernel_assert("size should be 2", value == 2);
	kernel_assert("should return true", list_get(list, 2, (void*)&value));
	kernel_assert("size should be 1", value == 1);

	kernel_assert("should return true", list_remove_tail(list));
	kernel_assert("size should be 2", list_size(list) == 2);
	kernel_assert("should return true", list_get(list, 0, (void*)&value));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("should return true", list_get(list, 1, (void*)&value));
	kernel_assert("size should be 2", value == 2);
	kernel_assert("should return false", !list_get(list, 2, (void*)&value));

	kernel_assert("should return true", list_get_tail(list, (void*)&value));
	kernel_assert("size should be 2", value == 2);
	kernel_assert("should return true", list_next(list, (void*)3, (void*)&value));
	kernel_assert("size should be 2", value == 2);

	kernel_assert("should return true", list_remove_tail(list));
	kernel_assert("should return true", list_remove_tail(list));
	kernel_assert("size should be 0", list_size(list) == 0);

	list_delete(list);
}

static void test_hashmap(mem_pool_info_t * const pool)
{
	uint32_t key_zero = 0;
	uint32_t values[10];
	for (uint8_t i = 1 ; i < 10 ; i++)
	{
		values[i] = i;
	}
	map_t * map = map_create(hash_basic_integer, pool, sizeof(uint32_t));
	kernel_assert("size match", map_capacity(map) == 65535);
	kernel_assert("size should be 0", map_size(map) == 0);

	kernel_assert("should return true", map_put(map, &key_zero, &values[0]));
	kernel_assert("size should be 1", map_size(map) == 1);
	for (uint8_t i = 1 ; i < 10 ; i++)
	{
		uint32_t key = i;
		kernel_assert("should return false", !map_contains_key(map, &key));
		kernel_assert("should return true", map_put(map, &key, &values[i]));
	}
	kernel_assert("size should be 10", map_size(map) == 10);
	for (uint8_t i = 0 ; i < 10 ; i++)
	{
		uint32_t key = i;
		uint32_t value = -1;
		kernel_assert("should return true", map_get(map, &key, (void*)&value));
		kernel_assert("should match value", i == value);
	}

	kernel_assert("should return true", map_remove(map, &key_zero));
	kernel_assert("should return false", !map_contains_key(map, &key_zero));
	kernel_assert("size should be 9", map_size(map) == 9);
	for (uint8_t i = 1 ; i < 10 ; i++)
	{
		uint32_t key = i;
		kernel_assert("should return true", map_contains_key(map, &key));
		kernel_assert("should return true", map_remove(map, &key));
		kernel_assert("should return false", !map_contains_key(map, &key));
	}
	kernel_assert("size should be 0", map_size(map) == 0);

	map_delete(map);
}

static void test_stack(mem_pool_info_t * const pool)
{
	stack_t * const stack = stack_create(pool);
	kernel_assert("size should be 0", stack_size(stack) == 0);

	kernel_assert("should return true", stack_push(stack, (void*)1));
	kernel_assert("should return true", stack_push(stack, (void*)2));
	kernel_assert("should return true", stack_push(stack, (void*)3));
	kernel_assert("size should be 3", stack_size(stack) == 3);

	uint32_t value = 0;
	kernel_assert("should return true", stack_front(stack, (void*)&value));
	kernel_assert("size should be 3", value == 3);

	kernel_assert("should return true", stack_get(stack, 1, (void*)&value));
	kernel_assert("size should be 2", value == 2);

	kernel_assert("should return true", stack_pop(stack, (void*)&value));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("size should be 3", stack_size(stack) == 2);

	// insert
	kernel_assert("should return true", stack_insert(stack, 2, (void*)3));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("size should be 3", stack_size(stack) == 3);
	kernel_assert("should return false", !stack_insert(stack, 5, (void*)6));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("size should be 3", stack_size(stack) == 3);

	stack_delete(stack);
}

static void test_queue(mem_pool_info_t * const pool)
{
	// remove
	queue_t * const queue = queue_create(pool);
	uint32_t value = 0;
	kernel_assert("should be 0", queue_size(queue) == 0);

	kernel_assert("should return true", queue_reorder_first(queue));
	kernel_assert("should return false", !queue_front(queue, (void*)&value));
	kernel_assert("should return false", !queue_remove(queue, (void*)0));

	kernel_assert("should return true", queue_push(queue, (void*)1));
	kernel_assert("should be 1", queue_size(queue) == 1);

	kernel_assert("should return true", queue_push(queue, (void*)2));
	kernel_assert("should return true", queue_push(queue, (void*)3));
	kernel_assert("should be 3", queue_size(queue) == 3);

	kernel_assert("should return true", queue_front(queue, &value));
	kernel_assert("should be 1", value == 1);

	kernel_assert("should return true", queue_reorder_first(queue));

	kernel_assert("should return true", queue_front(queue, &value));
	kernel_assert("should be 3", value == 3);

	kernel_assert("should return true", queue_pop(queue));
	kernel_assert("should be 2", queue_size(queue) == 2);
	kernel_assert("should return true", queue_front(queue, &value));
	kernel_assert("should be 2", value == 2);

	kernel_assert("should return true", queue_remove(queue, (void*)1));
	kernel_assert("should be 1", queue_size(queue) == 1);
	kernel_assert("should return true", queue_front(queue, &value));
	kernel_assert("should be 2", value == 2);

	queue_delete(queue);
}

void test_collections(void)
{
	mem_pool_info_t * const pool = mem_get_default_pool();

	const mem_t allocated = mem_get_allocd_size(pool);

	void * const test_base = mem_alloc(pool, TEST_POOL_SIZE);
	mem_pool_info_t * test_pool = NULL;
	mem_init_memory_pool((mem_t)test_base, TEST_POOL_SIZE, &test_pool);

	const mem_t collection_start_size = mem_get_allocd_size(test_pool);
	debug_print(COLLECTIONS, "Start allocated memory %x\n", collection_start_size);
	test_list(test_pool);
	debug_print(COLLECTIONS, "Linked List allocated memory %x\n", mem_get_allocd_size(test_pool));
	test_hashmap(test_pool);
	debug_print(COLLECTIONS, "Hash Map allocated memory %x\n", mem_get_allocd_size(test_pool));
	test_stack(test_pool);
	debug_print(COLLECTIONS, "Stack allocated memory %x\n", mem_get_allocd_size(test_pool));
	test_queue(test_pool);
	debug_print(COLLECTIONS, "Queue allocated memory %x\n", mem_get_allocd_size(test_pool));

	const mem_t collection_allocated = mem_get_allocd_size(test_pool);
	mem_free(pool, test_base);
	const mem_t main_allocated = mem_get_allocd_size(pool);

	debug_print(
			COLLECTIONS,
			"Collection memory still allocated %x, Main allocated was %x now %x\n",
			collection_allocated,
			allocated,
			main_allocated);

	kernel_assert("collection memory leak", collection_start_size == collection_allocated );
	kernel_assert("collection memory leak in parent pool", allocated == main_allocated);
}
#endif
