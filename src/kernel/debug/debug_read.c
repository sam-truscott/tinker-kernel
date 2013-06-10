/*
 * debug_read.c
 *
 *  Created on: 11 Aug 2011
 *      Author: sam
 */

#include "debug_read.h"

#include "../../arch/board_support.h"

char __debug_read(void)
{
	return __bsp_read_debug_char();
}
