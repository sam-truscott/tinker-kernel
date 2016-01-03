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

void print_stack_trace(process_t * const proc, const uint32_t frame_pointer)
{
	uint32_t * fp = (uint32_t*)frame_pointer;
	uint8_t limit = ISR_PRINT_STACKTRACE_LIMIT;

	while (*fp != 0 && --limit)
	{
		fp = (uint32_t*)*fp;
		if ((uint32_t)fp >= VIRTUAL_ADDRESS_SPACE(process_is_kernel(proc)))
		{
			uint32_t * real_fp;
			real_fp = (uint32_t*)process_virt_to_real(proc, (uint32_t)fp);
			error_print("0x%X -> 0x%X: ", fp, real_fp);
			fp = real_fp;
		}
		// FIXME - PPC only, not ARM
		uint32_t pc = *(fp + 4);

		/* TODO can we interrogate the symbol table?
		 * not without having a separate file or embedded ourself in */
		error_print("0x%X\n", pc);
	}
}

