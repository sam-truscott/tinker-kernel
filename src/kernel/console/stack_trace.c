/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "config.h"
#include "stack_trace.h"
#include "print_out.h"
#include "arch/tgt.h"

void __print_current_stack_trace(void)
{
	__print_stack_trace(__tgt_get_stack_pointer());
}

void __print_stack_trace(const uint32_t frame_pointer)
{
	const uint32_t * sp = (const uint32_t*)frame_pointer;
	uint32_t pc = 0;
	uint8_t limit = ISR_PRINT_STACKTRACE_LIMIT;

	while (*sp != 0 && --limit)
	{
		sp = (uint32_t*)*sp;
		pc = *(sp + 1);

		/* TODO can we interrogate the symbol table?
		 * not without having a separate file or embedded ourself in */
		__error_print("%X\n", pc);
	}
}

