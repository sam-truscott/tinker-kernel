/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kshell.h"
#if defined(KERNEL_SHELL)
#include "api/tinker_api_types.h"
#include "kernel/console/print_out.h"
#include "kernel/console/read_in.h"
#include "kernel/process/process_manager.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_memset.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/objects/obj_process.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/objects/obj_pipe.h"
#include "kernel/utils/collections/hashed_map.h"
#include "kernel/utils/collections/hashed_map_iterator.h"

#define MAX_LINE_INPUT 256

static char ksh_input_buffer[MAX_LINE_INPUT];

static uint8_t ksh_input_pointer;

static bool_t kshell_strcmp(const char * a, const char * b);

static void kshell_execute_command(const char* command);

static void kshell_process_list(void);

static void kshell_task_list(void);

static void kshell_object_table(void);

static char ksh_thread_states[9][255] =
{
		"ERROR  \0",
		"NCREATE\0",
		"IDLE   \0",
		"READY  \0",
		"RUNNING\0",
		"SYSTEM \0",
		"PAUSED \0",
		"WAITING\0",
		"TERMATE\0"
};

static char ksh_object_types[9][255] =
{
		"UNKNOWN\0",
		"OBJECT \0",
		"PROCESS\0",
		"THREAD \0",
		"PIPE   \0",
		"SEMAPHR\0",
		"SHRMEM \0",
		"CLOCK  \0",
		"TIMER  \0"
};

void kshell_start(void)
{
	ksh_input_pointer = 0;
	bool_t running = true;
	while (running)
	{
		ksh_input_buffer[ksh_input_pointer] = debug_read();
		if (ksh_input_buffer[ksh_input_pointer] == '\r'
				|| ksh_input_buffer[ksh_input_pointer] == '\n')
		{
			ksh_input_buffer[ksh_input_pointer] = '\0';
			if (ksh_input_pointer)
			{
				if (kshell_strcmp(ksh_input_buffer, "exit"))
				{
					running = false;
				}
				else
				{
					kshell_execute_command(ksh_input_buffer);
					ksh_input_pointer = 0;
					util_memset(ksh_input_buffer, 0, MAX_LINE_INPUT);
				}
			}
		}
		else
		{
			ksh_input_pointer++;
		}
	}
}

static void kshell_execute_command(const char* command)
{
	if (kshell_strcmp(command, "procs"))
	{
		kshell_process_list();
	}
	else if (kshell_strcmp(command, "tasks"))
	{
		kshell_task_list();
	}
	else if ( kshell_strcmp(command, "objects"))
	{
		kshell_object_table();
	}
	else
	{
		print_out("?\n");
	}
}

static bool_t kshell_strcmp(const char * a, const char * b)
{
	const uint32_t length = util_strlen(a, 65535);

	if (length != util_strlen(b, 65535))
	{
		return false;
	}

	uint32_t p;
	bool_t ok = true;

	for (p = 0 ; p < length && p < 65535 && ok ; p++)
	{
		if (a[p] != b[p])
		{
			ok = false;
		}
	}

	return ok;
}

static void kshell_process_list(void)
{
	process_list_it_t * list = NULL;
	process_t * proc = NULL;

	list = proc_list_procs();
	process_list_it_t_get(list, &proc);

	print_out("ProcessId\tThreads\tName\n");
	print_out("---------\t-------\t----\n");

	while (proc)
	{
		printp_out("\t%d", process_get_pid(proc));
		printp_out("\t%d", process_get_thread_count(proc));
		printp_out("\t%s\n", process_get_image(proc));
		if ( !process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
}

static void kshell_task_list(void)
{
	process_list_it_t * list = NULL;
	process_t * proc = NULL;

	list = proc_list_procs();
	process_list_it_t_get(list, &proc);

	while (proc)
	{
		thread_it_t * tlist = process_iterator(proc);
		thread_t  * t = NULL;

		thread_it_t_get(tlist, &t);

		printp_out("Process:\t%s\n", process_get_image(proc));
		print_out("Thread ID\tStack\tPri\tState\tEntry\tName\n");
		print_out("---------\t-----\t---\t-----\t-----\t----\n");

		while (t)
		{
			printp_out("\t%d", thread_get_tid(t));
			printp_out("\t0x%X", thread_get_stack_size(t));
			printp_out("\t%d", thread_get_priority(t));
			printp_out("\t%s", ksh_thread_states[thread_get_state(t)]);
			printp_out("\t%x", thread_get_entry_point(t));
			printp_out("\t%s", thread_get_name(t));
			print_out("\n");

			if (!thread_it_t_next(tlist, &t))
			{
				t = NULL;
			}
		}
		print_out("\n");

		thread_it_t_delete(tlist);

		if (!process_list_it_t_next(list, &proc))
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
}

static void kshell_object_table(void)
{
	process_list_it_t * list = NULL;
	process_t * proc = NULL;

	list = proc_list_procs();
	process_list_it_t_get(list, &proc);

	while (proc)
	{
		object_table_it_t * const it = obj_iterator(process_get_object_table(proc));

		printp_out("Process:\t%s\n", process_get_image(proc));
		print_out("ObjNo.\tType\t\n");
		print_out("------\t----\t\n");

		if (it)
		{
			object_t * obj = NULL;

			object_table_it_t_get(it, &obj);

			while (obj)
			{
				object_type_t type = obj_get_type(obj);

				printp_out("%d\t", obj_get_number(obj));
				printp_out("%s\t", ksh_object_types[type]);

				switch (type)
				{
					case UNKNOWN_OBJ:
					case OBJECT:
						break;

					case PROCESS_OBJ:
					{
						object_process_t * const p = obj_cast_process(obj);
						if (p)
						{
							printp_out("PId:\t%d", obj_process_pid(p));
						}
						else
						{
							print_out("Invalid Object");
						}
					}
						break;
					case THREAD_OBJ:
					{
						object_thread_t * const t = obj_cast_thread(obj);
						if (t)
						{
							thread_t * const thread = obj_get_thread(t);
							if (thread)
							{
								printp_out("Tid:\t%d\t", thread_get_tid(thread));
								printp_out("PriInherit: %d", thread_get_priority(thread));
							}
						}
						else
						{
							print_out("Invalid Object");
						}
					}
						break;
					case PIPE_OBJ:
						/* TODO KSHELL dump for Pipe */
						break;
					case SEMAPHORE_OBJ:
					{
						object_sema_t * const s = obj_cast_semaphore(obj);
						if (s)
						{
							printp_out("Count:\t%d\t", obj_get_sema_count(s));
							printp_out("Alloc:\t%d\t", obj_get_sema_alloc(s));
							printp_out("HighPri:%d", obj_get_sema_highest_priority(s));
						}
						else
						{
							print_out("Invalid Object");
						}
					}
						break;
					case SHARED_MEMORY_OBJ:
						/* TODO KSHELL dump for Shared Memory */
						break;
					case TIMER_OBJ:
						/* TODO KSHELL dump for Timer */
						break;
				}

				print_out("\n");

				if ( !object_table_it_t_next(it, &obj))
				{
					obj = NULL;
				}
			}

			print_out("\n");

			object_table_it_t_delete(it);
		}

		if (!process_list_it_t_next(list, &proc))
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
}
#endif /* KERNEL_SHELL */
