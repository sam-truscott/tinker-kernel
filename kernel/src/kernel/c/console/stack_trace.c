/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "config.h"
#include "console/stack_trace.h"
#include "console/print_out.h"
#include "tgt.h"

void print_stack_trace(process_t * const proc, const mem_t frame_pointer)
{
	mem_t * fp = (mem_t*)frame_pointer;
	uint8_t limit = ISR_PRINT_STACKTRACE_LIMIT;

	while (*fp != 0 && --limit)
	{
		fp = (mem_t*)*fp;
		if ((mem_t)fp >= VIRTUAL_ADDRESS_SPACE(process_is_kernel(proc)))
		{
			mem_t * real_fp;
			real_fp = (mem_t*)process_virt_to_real(proc, (mem_t)fp);
			error_print("0x%X -> 0x%X: ", fp, real_fp);
			fp = real_fp;
		}
		// FIXME - PPC only, not ARM
		mem_t pc = *(fp + 4);

		/* TODO can we interrogate the symbol table?
		 * not without having a separate file or embedded ourself in */
		error_print("0x%X\n", pc);
	}
}

