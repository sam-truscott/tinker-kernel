/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
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

