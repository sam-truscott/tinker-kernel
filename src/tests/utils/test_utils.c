/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "kernel/kernel_assert.h"
#include "kernel/utils/util_case.h"
#include "kernel/utils/util_i_to_a.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/utils/util_memset.h"
#include "kernel/utils/util_reverse_string.h"
#include "kernel/utils/util_streq.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_trim.h"

static void test_streq(void)
{
	kernel_assert("should be true", util_streq(NULL, NULL, 1));
	kernel_assert("should be true", util_streq(NULL, NULL, 2));
	kernel_assert("should be true", util_streq("a", "a", 1));
	kernel_assert("should be false", !util_streq("a", "b", 1));
	kernel_assert("should be true", util_streq("string", "string", 6));
	kernel_assert("should be false", !util_streq("string", "trings", 6));
	kernel_assert("should be false", !util_streq("string", NULL, 6));
	kernel_assert("should be false", !util_streq(NULL, "string", 6));
}

static void test_case(void)
{
	char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	util_to_lower(NULL);
	util_to_lower(buffer);
	kernel_assert("should be true", util_streq(buffer, "", 0));

	buffer[0] = 'A';
	buffer[1] = 'B';
	buffer[2] = 'C';
	buffer[3] = '-';
	buffer[5] = 'A';
	util_to_lower(buffer);
	kernel_assert("should be true", util_streq("abc-", NULL, 4));
	util_to_upper(buffer);
	kernel_assert("should be true", util_streq("ABC-", NULL, 4));
}

void test_utils(void)
{
	test_streq();
	test_case();
}


