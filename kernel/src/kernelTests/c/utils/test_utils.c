/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "kernel_assert.h"
#include "utils/util_case.h"
#include "utils/util_i_to_a.h"
#include "utils/util_memcpy.h"
#include "utils/util_memset.h"
#include "utils/util_reverse_string.h"
#include "utils/util_streq.h"
#include "utils/util_strlen.h"
#include "utils/util_trim.h"

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
	kernel_assert("should be false", !util_streq("abc-", NULL, 4));
	kernel_assert("should be false", !util_streq("ABC-", NULL, 4));

	buffer[0] = 'A';
	buffer[1] = 'B';
	buffer[2] = 'C';
	buffer[3] = '-';
	buffer[5] = 'A';
	util_to_lower(buffer);
	kernel_assert("should be true", util_streq("abc-", buffer, 4));
	util_to_upper(buffer);
	kernel_assert("should be true", util_streq("ABC-", buffer, 4));
}

static void test_memcpy_memset(void)
{
	char source[11];
	char dest[11];
	source[10] = dest[10] = 0;
	for (uint8_t i = 0 ; i < 10 ; i++)
	{
		source[i] = '0' + i;
	}
	kernel_assert("should be true", util_streq("0123456789", source, 10));
	util_memset(dest, '0', 10);
	kernel_assert("should be true", util_streq("0000000000", dest, 10));
	util_memcpy(dest, source, 10);
	kernel_assert("should be true", util_streq("0123456789", dest, 10));
	util_memset(dest, '0', 10);
	util_memcpy(dest, source, 5);
	kernel_assert("should be true", util_streq("0123400000", dest, 10));
	util_memset(dest, '0', 10);
	util_memcpy(dest+5, source+5, 5);
	kernel_assert("should be true", util_streq("0000056789", dest, 10));
}

static void test_strlen(void)
{
	kernel_assert("should be 0", 0 == util_strlen(NULL, 1));
	kernel_assert("should be 0", 0 == util_strlen("", 1));
	kernel_assert("should be 1", 1 == util_strlen("A", 1));
	kernel_assert("should be 1", 1 == util_strlen("A\0B", 3));
	kernel_assert("should be 2", 2 == util_strlen("AB", 2));
}

static void test_trim(void)
{
	uint32_t length = util_trim(NULL, 0);
	kernel_assert("should be true", length == 0);

	char buffer[4];
	util_memset(buffer, 0, 4);

	length = util_trim(buffer, 4);
	kernel_assert("should be true", util_streq("", buffer, length));

	buffer[0] = '0';
	length = util_trim(buffer, 1);
	kernel_assert("should be true", util_streq("", buffer, length));

	buffer[0] = 'A';
	length = util_trim(buffer, 1);
	kernel_assert("should be true", util_streq("A", buffer, length));

	buffer[0] = 'A';
	buffer[1] = '0';
	buffer[2] = '\0';
	length = util_trim(buffer, 2);
	kernel_assert("should be true", util_streq("A", buffer, length));

	buffer[0] = 'A';
	buffer[1] = '0';
	buffer[2] = '0';
	buffer[3] = '\0';
	length = util_trim(buffer, 3);
	kernel_assert("should be true", util_streq("A", buffer, length));

	buffer[0] = 'A';
	buffer[1] = '0';
	buffer[2] = 'A';
	buffer[3] = '\0';
	length = util_trim(buffer, 3);
	kernel_assert("should be true", util_streq("A0A", buffer, length));
}

static void test_reverse(void)
{
	char buffer[11];
	char buffer2[11];
	util_memset(buffer, 0, 11);
	util_memset(buffer2, 0, 11);

	util_reverse_string(NULL, 10, buffer);
	kernel_assert("should be true", util_streq("", buffer, 10));

	util_memcpy(buffer2, "0123456789", 10);
	util_reverse_string(buffer2, 10, buffer);
	kernel_assert("should be true", util_streq("9876543210", buffer, 10));
}

static void test_i_to_a(void)
{
	char buffer[11];
	util_memset(buffer, 0, 11);

	util_i_to_a(0, buffer, 10);
	kernel_assert("should be true", util_streq("0", buffer, 10));

	util_memset(buffer, 0, 11);
	util_i_to_a(123456789, buffer, 10);
	kernel_assert("should be true", util_streq("123456789", buffer, 10));

	util_memset(buffer, 0, 11);
	util_i_to_a(-123456789, buffer, 10);
	kernel_assert("should be true", util_streq("-123456789", buffer, 10));

	util_memset(buffer, 0, 11);
	util_i_to_h(123456789, buffer, 10);
	kernel_assert("should be true", util_streq("75BCD15", buffer, 10));
}

void test_utils(void)
{
	test_streq();
	test_case();
	test_memcpy_memset();
	test_strlen();
	test_trim();
	test_reverse();
	test_i_to_a();
}


