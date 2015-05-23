/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "kernel/kernel_assert.h"
#include "kernel/kernel_panic.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/utils/collections/unbounded_list.h"
#include "kernel/utils/collections/hashed_map.h"
#include "kernel/utils/collections/stack.h"
#include "kernel/utils/collections/unbounded_queue.h"

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
	(void)pool;
}

static void test_stack(mem_pool_info_t * const pool)
{
	(void)pool;
}

static void test_queue(mem_pool_info_t * const pool)
{
	(void)pool;
}

void test_collections(void)
{
	mem_pool_info_t * const pool = mem_get_default_pool();
	test_list(pool);
	test_hashmap(pool);
	test_stack(pool);
	test_queue(pool);
}
