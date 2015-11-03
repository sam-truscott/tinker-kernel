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

#define SCRATCH_SIZE 64 // TODO config

static tinker_pipe_t kernel_in_pipe;
static char * buffer;
static char * write_pointer;
static char * read_pointer;
static thread_t * in_thread;
static scheduler_t * scheduler;

static void kernel_in_isr(void);

void kernel_in_initialise(scheduler_t * const sch)
{
	scheduler = sch;
#if defined (DEBUG_IN)
	tinker_debug("System: Creating input pipe...\n");
#endif
	const error_t result = tinker_create_pipe(
			&kernel_in_pipe,
			"in",
			PIPE_SEND,
			4,
			256);
	kernel_assert("Kernel couldn't create input pipe", result == NO_ERROR);
#if defined (DEBUG_IN)
	{
		char buff[255];
		util_memset(buff, 0, 255);
		util_strcpy(buff, "System: Input pipe is called '", 255);
		util_strcat(buff, "in", 255); // TODO config
		util_strcat(buff, "' with object id ", 255);
		char kin[8];
		util_memset(kin, 0, 8);
		util_i_to_a(kenrel_input, kin, 8);
		util_strcat(buff, kin, 255);
		util_strcat(buff, "\n", 255);
		tinker_debug(buff);
	}
#endif

	// get a buffer to store temp data
	write_pointer = read_pointer = buffer = mem_alloc(mem_get_default_pool(), SCRATCH_SIZE);

	// thread to write to the pipes on
	proc_create_thread(
				kernel_get_process(),
				"kernel_in",
				kernel_in_isr,
				1,
				0x400,
				0,
				NULL,
				&in_thread);
}


void kernel_in_write(char * const buffer_to_write, const uint16_t size)
{
	for (uint16_t byte = 0 ; byte < size ; byte++)
	{
		*(write_pointer++) = buffer_to_write[byte];
		if (write_pointer == (buffer + SCRATCH_SIZE))
		{
			write_pointer = buffer;
		}
	}
#if defined(IN_DEBUGGING)
	printp_out("IN: wp %x\n", write_pointer);
#endif
	thread_set_state(in_thread, THREAD_READY);
	sch_notify_resume_thread(scheduler, in_thread);
}

static void kernel_in_isr(void)
{
	while(1)
	{
		while (read_pointer != write_pointer)
		{
#if defined(IN_DEBUGGING)
			printp_out("IN: Sending\n");
#endif
			tinker_send_message(
					kernel_in_pipe,
					PIPE_TX_SEND_AVAILABLE,
					read_pointer,
					1,
					false);
			read_pointer++;
			if (read_pointer == (buffer + SCRATCH_SIZE))
			{
				read_pointer = buffer;
			}
#if defined(IN_DEBUGGING)
			printp_out("IN: rp %x\n", read_pointer);
#endif
		}
#if defined(IN_DEBUGGING)
		printp_out("IN: sleeping\n");
#endif
		tinker_thread_wait();
	}
}
