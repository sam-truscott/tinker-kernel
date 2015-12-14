/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "unit_tests.h"
#include "board_support.h"

int main(int argc, char * argv[])
{
	bsp_initialise();
	mem_initialise(bsp_get_usable_memory_start(), bsp_get_usable_memory_end());

	(void)argc; // UNUSED
	(void)argv; // UNUSED
	run_unit_tests();
	return 0;
}
