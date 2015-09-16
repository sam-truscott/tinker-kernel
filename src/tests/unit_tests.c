/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "unit_tests.h"
#include "collections/test_collections.h"
#include "utils/test_utils.h"
#include "memory/test_memory.h"
#include "scheduler/test_scheduler.h"

#if defined(UNIT_TESTS)
void run_unit_tests(void)
{
	test_utils();
	test_memory();
	test_collections();
	test_scheduler();
}
#endif
