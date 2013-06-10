/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "debug_stack_trace.h"
#include "debug_print.h"
#include "../../arch/tgt.h"

void __debug_print_current_stack_trace(void)
{
	__debug_print_stack_trace(__tgt_get_stack_pointer());
}

void __debug_print_stack_trace(uint32_t frame_pointer)
{
	uint32_t * sp = (uint32_t*)frame_pointer;
	uint32_t pc;

	while (*sp != 0)
	{
		sp = (uint32_t*)*sp;
		pc = *(sp + 1);

		/* TODO can we interrogate the symbol table?
		 * not without having a separate file or embedded ourself in */
		__debug_print("%X\n", pc);
	}
}

