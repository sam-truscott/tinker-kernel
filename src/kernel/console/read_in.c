/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "read_in.h"

#include "arch/board_support.h"

char __debug_read(void)
{
	return __bsp_read_debug_char();
}
