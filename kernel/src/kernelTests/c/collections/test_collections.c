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
#include "utils/collections/stack.h"
#include "utils/collections/unbounded_queue.h"

#if defined(UNIT_TESTS)

#define TEST_POOL_SIZE 256 * 1024

UNBOUNDED_LIST_TYPE(test_list_t)
UNBOUNDED_LIST_INTERNAL_TYPE(test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_INITIALISE(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_INITIALISE(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_ADD(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_ADD(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_INSERT(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_INSERT(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_REMOVE(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_REMOVE(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_REMOVE_ITEM(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_REMOVE_TAIL(static, test_list_t)
UNBOUNDED_LIST_BODY_REMOVE_TAIL(static, test_list_t)
UNBOUNDED_LIST_SPEC_HEAD_TO_TAIL(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_HEAD_TO_TAIL(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_GET(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_GET(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_GET_TAIL(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_GET_TAIL(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_NEXT(static, test_list_t, uint32_t)
UNBOUNDED_LIST_BODY_NEXT(static, test_list_t, uint32_t)
UNBOUNDED_LIST_SPEC_SIZE(static, test_list_t)
UNBOUNDED_LIST_BODY_SIZE(static, test_list_t)

#define MAP_SIZE 1024
#define MAP_BUCKET_SIZE 16
HASH_MAP_TYPE_T(test_map_t)
HASH_MAP_INTERNAL_TYPE_T(test_map_t, uint32_t, uint32_t, MAP_SIZE, MAP_BUCKET_SIZE)
HASH_FUNCS_VALUE(test_map_t, uint32_t)
HASH_MAP_SPEC_INITALISE(static, test_map_t)
HASH_MAP_BODY_INITALISE(static, test_map_t, MAP_SIZE, MAP_BUCKET_SIZE)
HASH_MAP_SPEC_SIZE(static, test_map_t)
HASH_MAP_BODY_SIZE(static, test_map_t)
HASH_MAP_SPEC_PUT(static, test_map_t, uint32_t, uint32_t)
HASH_MAP_BODY_PUT(static, test_map_t, uint32_t, uint32_t, MAP_SIZE, MAP_BUCKET_SIZE)
HASH_MAP_SPEC_GET(static, test_map_t, uint32_t, uint32_t)
HASH_MAP_BODY_GET(static, test_map_t, uint32_t, uint32_t, MAP_BUCKET_SIZE)

HASH_MAP_SPEC_REMOVE(static, test_map_t, uint32_t, uint32_t)
HASH_MAP_BODY_REMOVE(static, test_map_t, uint32_t, MAP_BUCKET_SIZE)
HASH_MAP_SPEC_CAPACITY(static, test_map_t)
HASH_MAP_BODY_CAPACITY(static, test_map_t, MAP_SIZE)
HASH_MAP_SPEC_CONTAINS_KEY(static, test_map_t, uint32_t)
HASH_MAP_BODY_CONTAINS_KEY(static, test_map_t, uint32_t, MAP_BUCKET_SIZE)

STACK_TYPE(test_stack_t)
STACK_INTERNAL_TYPE(test_stack_t, uint32_t)
STACK_SPEC_CREATE(static, test_stack_t, uint32_t)
STACK_BODY_CREATE(static, test_stack_t, uint32_t)
STACK_SPEC_PUSH(static, test_stack_t, uint32_t)
STACK_BODY_PUSH(static, test_stack_t, uint32_t)
STACK_SPEC_POP(static, test_stack_t, uint32_t)
STACK_BODY_POP(static, test_stack_t, uint32_t)
STACK_SPEC_FRONT(static, test_stack_t, uint32_t)
STACK_BODY_FRONT(static, test_stack_t, uint32_t)
STACK_SPEC_SIZE(static, test_stack_t)
STACK_BODY_SIZE(static, test_stack_t)
STACK_SPEC_GET(static, test_stack_t, uint32_t)
STACK_BODY_GET(static, test_stack_t, uint32_t)
STACK_SPEC_INSERT(static, test_stack_t, uint32_t)
STACK_BODY_INSERT(static, test_stack_t, uint32_t)
STACK_SPEC_DELETE(static, test_stack_t, uint32_t)
STACK_BODY_DELETE(static, test_stack_t, uint32_t)

UNBOUNDED_QUEUE_TYPE(test_queue_t)
UNBOUNDED_QUEUE_INTERNAL_TYPE(test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_CREATE(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_CREATE(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_PUSH(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_PUSH(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_POP(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_POP(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_FRONT(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_FRONT(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_REORDER_FIRST(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_REORDER_FIRST(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_SIZE(static, test_queue_t)
UNBOUNDED_QUEUE_BODY_SIZE(static, test_queue_t)
UNBOUNDED_QUEUE_SPEC_REMOVE(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_REMOVE(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_SPEC_DELETE(static, test_queue_t, uint32_t)
UNBOUNDED_QUEUE_BODY_DELETE(static, test_queue_t, uint32_t)

static void test_list(mem_pool_info_t * const pool)
{
	test_list_t list;
	test_list_t_initialise(&list, pool);

	// add and remove
	kernel_assert("size should be 0", test_list_t_size(&list) == 0);
	kernel_assert("should return true", test_list_t_add(&list, 0));
	kernel_assert("size should be 1", test_list_t_size(&list) == 1);
	uint32_t value = 1;
	kernel_assert("should return true", test_list_t_get(&list, 0, &value));
	kernel_assert("size should be 0", value == 0);
	kernel_assert("should return true", test_list_t_remove(&list, 0));
	kernel_assert("size should be 0", test_list_t_size(&list) == 0);

	// add and remove
	kernel_assert("should return true", test_list_t_add(&list, 1));
	kernel_assert("should return true", test_list_t_add(&list, 2));
	kernel_assert("should return true", test_list_t_add(&list, 3));
	kernel_assert("size should be 3", test_list_t_size(&list) == 3);
	kernel_assert("should return true", test_list_t_get(&list, 0, &value));
	kernel_assert("size should be 3", test_list_t_size(&list) == 3);
	kernel_assert("should return true", test_list_t_remove(&list, 0));
	kernel_assert("should return true", test_list_t_get(&list, 0, &value));
	kernel_assert("size should be 2", test_list_t_size(&list) == 2);
	kernel_assert("should return true", test_list_t_remove(&list, 0));
	kernel_assert("should return true", test_list_t_get(&list, 0, &value));
	kernel_assert("should return true", test_list_t_remove(&list, 0));
	kernel_assert("size should be 0", test_list_t_size(&list) == 0);

	kernel_assert("should return true", test_list_t_add(&list, 1));
	kernel_assert("size should be 0", test_list_t_size(&list) == 1);
	kernel_assert("should return true", test_list_t_remove_item(&list, 1));
	kernel_assert("size should be 0", test_list_t_size(&list) == 0);

	kernel_assert("should return true", test_list_t_insert(&list, 0, 1));
	kernel_assert("should return true", test_list_t_insert(&list, 1, 2));
	kernel_assert("should return true", test_list_t_insert(&list, 2, 3));
	kernel_assert("size should be 3", test_list_t_size(&list) == 3);
	kernel_assert("should return false", !test_list_t_insert(&list, 6, 6));
	kernel_assert("size should be 3", test_list_t_size(&list) == 3);

	kernel_assert("should return true", test_list_t_head_to_tail(&list));
	kernel_assert("size should be 3", test_list_t_size(&list) == 3);
	kernel_assert("should return true", test_list_t_get(&list, 0, &value));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("should return true", test_list_t_get(&list, 1, &value));
	kernel_assert("size should be 2", value == 2);
	kernel_assert("should return true", test_list_t_get(&list, 2, &value));
	kernel_assert("size should be 1", value == 1);

	kernel_assert("should return true", test_list_t_remove_tail(&list));
	kernel_assert("size should be 2", test_list_t_size(&list) == 2);
	kernel_assert("should return true", test_list_t_get(&list, 0, &value));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("should return true", test_list_t_get(&list, 1, &value));
	kernel_assert("size should be 2", value == 2);
	kernel_assert("should return false", !test_list_t_get(&list, 2, &value));

	kernel_assert("should return true", test_list_t_get_tail(&list, &value));
	kernel_assert("size should be 2", value == 2);
	kernel_assert("should return true", test_list_t_next(&list, 3, &value));
	kernel_assert("size should be 2", value == 2);

	kernel_assert("should return true", test_list_t_remove_tail(&list));
	kernel_assert("should return true", test_list_t_remove_tail(&list));
	kernel_assert("size should be 0", test_list_t_size(&list) == 0);
}

static void test_hashmap(mem_pool_info_t * const pool)
{
	test_map_t map;
	test_map_t_initialise(&map, hash_basic_integer, hash_equal_integer, true, pool);
	kernel_assert("size match", test_map_t_capacity(&map) == MAP_SIZE);
	kernel_assert("size should be 0", test_map_t_size(&map) == 0);
	kernel_assert("should return true", test_map_t_put(&map, 0, 0));
	kernel_assert("size should be 1", test_map_t_size(&map) == 1);
	for (uint8_t i = 1 ; i < 10 ; i++)
	{
		kernel_assert("should return false", !test_map_t_contains_key(&map, i));
		kernel_assert("should return true", test_map_t_put(&map, i, i));
	}
	kernel_assert("size should be 10", test_map_t_size(&map) == 10);
	for (uint8_t i = 0 ; i < 10 ; i++)
	{
		uint32_t value = -1;
		kernel_assert("should return true", test_map_t_get(&map, i, &value));
		kernel_assert("should match value", i == value);
	}

	kernel_assert("should return true", test_map_t_remove(&map, 0));
	kernel_assert("should return false", !test_map_t_contains_key(&map, 0));
	kernel_assert("size should be 9", test_map_t_size(&map) == 9);
	for (uint8_t i = 1 ; i < 10 ; i++)
	{
		kernel_assert("should return true", test_map_t_contains_key(&map, i));
		kernel_assert("should return true", test_map_t_remove(&map, i));
		kernel_assert("should return false", !test_map_t_contains_key(&map, i));
	}
	kernel_assert("size should be 0", test_map_t_size(&map) == 0);
}

static void test_stack(mem_pool_info_t * const pool)
{
	test_stack_t * const stack = test_stack_t_create(pool);
	kernel_assert("size should be 0", test_stack_t_size(stack) == 0);

	kernel_assert("should return true", test_stack_t_push(stack, 1));
	kernel_assert("should return true", test_stack_t_push(stack, 2));
	kernel_assert("should return true", test_stack_t_push(stack, 3));
	kernel_assert("size should be 3", test_stack_t_size(stack) == 3);

	uint32_t value = 0;
	kernel_assert("should return true", test_stack_t_front(stack, &value));
	kernel_assert("size should be 3", value == 3);

	kernel_assert("should return true", test_stack_t_get(stack, 1, &value));
	kernel_assert("size should be 2", value == 2);

	kernel_assert("should return true", test_stack_t_pop(stack, &value));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("size should be 3", test_stack_t_size(stack) == 2);

	// insert
	kernel_assert("should return true", test_stack_t_insert(stack, 2, 3));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("size should be 3", test_stack_t_size(stack) == 3);
	kernel_assert("should return false", !test_stack_t_insert(stack, 5, 6));
	kernel_assert("size should be 3", value == 3);
	kernel_assert("size should be 3", test_stack_t_size(stack) == 3);

	test_stack_t_delete(stack);
}

static void test_queue(mem_pool_info_t * const pool)
{
	// remove
	test_queue_t * const queue = test_queue_t_create(pool);
	uint32_t value = 0;
	kernel_assert("should be 0", test_queue_t_size(queue) == 0);

	kernel_assert("should return true", test_queue_t_reorder_first(queue));
	kernel_assert("should return false", !test_queue_t_front(queue, &value));
	kernel_assert("should return false", !test_queue_t_remove(queue, 0));

	kernel_assert("should return true", test_queue_t_push(queue, 1));
	kernel_assert("should be 1", test_queue_t_size(queue) == 1);

	kernel_assert("should return true", test_queue_t_push(queue, 2));
	kernel_assert("should return true", test_queue_t_push(queue, 3));
	kernel_assert("should be 3", test_queue_t_size(queue) == 3);

	kernel_assert("should return true", test_queue_t_front(queue, &value));
	kernel_assert("should be 1", value == 1);

	kernel_assert("should return true", test_queue_t_reorder_first(queue));

	kernel_assert("should return true", test_queue_t_front(queue, &value));
	kernel_assert("should be 3", value == 3);

	kernel_assert("should return true", test_queue_t_pop(queue));
	kernel_assert("should be 2", test_queue_t_size(queue) == 2);
	kernel_assert("should return true", test_queue_t_front(queue, &value));
	kernel_assert("should be 2", value == 2);

	kernel_assert("should return true", test_queue_t_remove(queue, 1));
	kernel_assert("should be 1", test_queue_t_size(queue) == 1);
	kernel_assert("should return true", test_queue_t_front(queue, &value));
	kernel_assert("should be 2", value == 2);

	test_queue_t_delete(queue);
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
