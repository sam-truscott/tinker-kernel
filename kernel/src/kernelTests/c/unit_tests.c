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
#include "process/test_proc_list.h"

#if defined(UNIT_TESTS)
void run_unit_tests(void)
{
	test_utils();
	test_memory();
	test_collections();
	test_scheduler();
	test_proc_list();
}
#endif
