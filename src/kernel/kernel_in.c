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
#include "kernel/objects/obj_pipe.h"

static object_pipe_t * kernel_in_pipe;

void kernel_in_initialise(void)
{
	debug_print("System: Creating input pipe...\n");
	object_number_t kenrel_input = INVALID_OBJECT_ID;
	process_t * const kernel_process = kernel_get_process();
	const error_t result = obj_create_pipe(
			kernel_process,
			&kenrel_input,
			"in",	// TODO CONFIG
			PIPE_SEND,
			1, 	// TODO CONFIG
			256);	// TODO CONFIG
	kernel_assert("Kernel couldn't create input pipe", result == NO_ERROR);
	debug_print("System: Input pipe is called '%s' with object id %d\n", "in", kenrel_input);

	kernel_in_pipe = obj_cast_pipe(
			obj_get_object(
					process_get_object_table(kernel_process),
					kenrel_input));
}

void kernel_in_write(char * const buffer, const uint16_t size)
{
	obj_pipe_send_message(
			kernel_in_pipe,
			NULL,
			PIPE_TX_SEND_AVAILABLE,
			buffer,
			size,
			false);
}
