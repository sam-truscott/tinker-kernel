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
#include "kernel/scheduler/scheduler.h"

void __print_current_stack_trace(void)
{
	__print_stack_trace(__tgt_get_stack_pointer());
}

void __print_stack_trace(const uint32_t frame_pointer)
{
	const __process_t * const proc = __thread_get_parent(__sch_get_current_thread());
	const uint32_t * sp = (const uint32_t*)frame_pointer;
	uint32_t pc = 0;
	uint8_t limit = ISR_PRINT_STACKTRACE_LIMIT;

	while (*sp != 0 && --limit)
	{
		sp = (uint32_t*)*sp;
		if ((uint32_t)sp >= VIRTUAL_ADDRESS_SPACE)
		{
			const uint32_t * real_sp = sp;
			real_sp = (const uint32_t*)__process_virt_to_real(proc, (uint32_t)sp);
			__error_print("0x%X -> 0x%X: ", sp, real_sp);
			sp = real_sp;
		}
		pc = *(sp + 1);

		/* TODO can we interrogate the symbol table?
		 * not without having a separate file or embedded ourself in */
		__error_print("0x%X\n", pc);
	}
}

