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
#include "kernel/process/process_manager.h"

static object_pipe_t * kernel_in_pipe;
static char * write_pos;
static char * read_pos;
static char * buffer;
static thread_t * in_thread;

static void kernel_in_process(void);

void kernel_in_initialise(void)
{
#if defined (DEBUG_IN)
	debug_print("System: Creating input pipe...\n");
#endif
	object_number_t kenrel_input = INVALID_OBJECT_ID;
	process_t * const kernel_process = kernel_get_process();
	const error_t result = obj_create_pipe(
			kernel_process,
			&kenrel_input,
			"in",	// TODO CONFIG
			PIPE_SEND,
			4, 	// TODO CONFIG
			256);	// TODO CONFIG
	kernel_assert("Kernel couldn't create input pipe", result == NO_ERROR);
#if defined (DEBUG_IN)
	debug_print("System: Input pipe is called '%s' with object id %d\n", "in", kenrel_input);
#endif

	kernel_in_pipe = obj_cast_pipe(
			obj_get_object(
					process_get_object_table(kernel_process),
					kenrel_input));

	proc_create_thread(
			kernel_get_process(),
			"in",
			kernel_in_process,
			1,
			0x100,
			0,
			NULL,
			&in_thread);
	thread_set_state(in_thread, THREAD_PAUSED);

	buffer = mem_alloc(mem_get_default_pool(), 1024);
	read_pos = buffer;
	write_pos = buffer;
#if defined (DEBUG_IN)
	debug_print("buffer at b=%x, r=%x w=%x\n", buffer, read_pos, write_pos);
#endif
}

void kernel_in_process(void)
{
	// TODO no need for this - pipes should be safe
	while (true)
	{
		uint32_t size;
		if (write_pos < read_pos)
		{
			size = ((buffer + 1024) - read_pos) + (write_pos - buffer);
		}
		else
		{
			size = (write_pos - read_pos);
		}
#if defined (DEBUG_IN)
		debug_print("reading, r=%x, w=%x, size=%d\n", read_pos, write_pos, size);
#endif
		while (size--)
		{
#if defined (DEBUG_IN)
			debug_print("writing %d\n", read_pos);
#endif
			obj_pipe_send_message(
					kernel_in_pipe,
					NULL,
					PIPE_TX_SEND_AVAILABLE,
					read_pos++,
					1,
					false);
			if (read_pos == (buffer + 1024))
			{
				read_pos = buffer;
			}
		}
#if defined (DEBUG_IN)
		debug_print("done reading\n");
#endif
		thread_set_state(in_thread, THREAD_PAUSED);
		tinker_wait_for_interrupt();
	}
}

void kernel_in_write(char * const buffer, const uint16_t size)
{
	// can't write in isr space
	// create a new process that runs and then pauses
#if defined (DEBUG_IN)
	debug_print("write %d from %x\n", size, buffer);
#endif
	uint16_t todo = size;
	char * src = buffer;
	while (todo--)
	{
		*(write_pos++) = *(src++);
		if (write_pos == (buffer + 1024))
		{
			write_pos = buffer;
		}
	}
#if defined (DEBUG_IN)
	debug_print("write done, w=%x\n", write_pos);
#endif
	thread_set_state(in_thread, THREADY_READY);
}
