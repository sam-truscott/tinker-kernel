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

char debug_read(void)
{
	return bsp_read_debug_char();
}
