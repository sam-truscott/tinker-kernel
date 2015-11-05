/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel_in.h"
#include "kernel/kernel_initialise.h"
#include "kernel/kernel_assert.h"
#include "kernel/utils/util_strcpy.h"
#include "kernel/utils/util_strcat.h"
#include "kernel/utils/util_i_to_a.h"

void kernel_in_initialise(registry_t * const reg)
{
#if defined (DEBUG_IN)
	tinker_debug("System: Creating input pipe...\n");
#endif
	object_number_t kernel_in_pipe = INVALID_OBJECT_ID;
	const error_t result = obj_create_pipe(
			reg,
			kernel_get_process(),
			&kernel_in_pipe,
			"in",
			PIPE_SEND,
			4,
			256);
	kernel_assert("Kernel couldn't create input pipe", result == NO_ERROR);
#if defined (DEBUG_IN)
	{
		printp_out("System: Input pipe is called '%s' with object id %d\n", "in", kenrel_input);
	}
#endif
}
