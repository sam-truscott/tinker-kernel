/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kshell.h"
#if defined(__KERNEL_SHELL)
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

static char __ksh_input_buffer[MAX_LINE_INPUT];

static uint8_t __ksh_input_pointer;

static bool_t __kshell_strcmp(const char * a, const char * b);

static void __kshell_execute_command(const char* command);

static void __kshell_process_list(void);

static void __kshell_task_list(void);

static void __kshell_object_table(void);

static char __ksh_thread_states[9][255] =
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

static char __ksh_object_types[9][255] =
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

void __kshell_start(void)
{
	__ksh_input_pointer = 0;
	bool_t running = true;
	while(running)
	{
		__ksh_input_buffer[__ksh_input_pointer] = __debug_read();
		if ( __ksh_input_buffer[__ksh_input_pointer] == '\r'
				|| __ksh_input_buffer[__ksh_input_pointer] == '\n' )
		{
			__ksh_input_buffer[__ksh_input_pointer] = '\0';
			if (__ksh_input_pointer)
			{
				if (__kshell_strcmp(__ksh_input_buffer, "exit"))
				{
					running = false;
				}
				else
				{
					__kshell_execute_command(__ksh_input_buffer);
					__ksh_input_pointer = 0;
					memset(__ksh_input_buffer, 0, MAX_LINE_INPUT);
				}
			}
		}
		else
		{
			__ksh_input_pointer++;
		}
	}
}

static void __kshell_execute_command(const char* command)
{
	if (__kshell_strcmp(command, "procs"))
	{
		__kshell_process_list();
	}
	else if (__kshell_strcmp(command, "tasks"))
	{
		__kshell_task_list();
	}
	else if ( __kshell_strcmp(command, "objects"))
	{
		__kshell_object_table();
	}
	else
	{
		__print_out("?\n");
	}
}

static bool_t __kshell_strcmp(const char * a, const char * b)
{
	const uint32_t l = __util_strlen(a, 65535);

	if ( l != __util_strlen(b, 65535) )
	{
		return false;
	}

	uint32_t p;
	bool_t ok = true;

	for(p = 0 ; p < l && p < 65535 && ok ; p++) {
		if ( a[p] != b[p])
		{
			ok = false;
		}
	}

	return ok;
}

static void __kshell_process_list(void)
{
	__process_list_it_t * list = NULL;
	__process_t * proc = NULL;

	list = __proc_list_procs();
	__process_list_it_t_get(list, &proc);

	__print_out("ProcessId\tThreads\tName\n");
	__print_out("---------\t-------\t----\n");

	while( proc )
	{
		__printp_out("\t%d", __process_get_pid(proc));
		__printp_out("\t%d", __process_get_thread_count(proc));
		__printp_out("\t%s\n", __process_get_image(proc));
		if ( !__process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	__process_list_it_t_delete(list);
}

static void __kshell_task_list(void)
{
	__process_list_it_t * list = NULL;
	__process_t * proc = NULL;

	list = __proc_list_procs();
	__process_list_it_t_get(list, &proc);

	while( proc )
	{
		__thread_it_t * tlist = __process_iterator(proc);
		__thread_t  * t = NULL;

		__thread_it_t_get(tlist, &t);

		__printp_out("Process:\t%s\n", __process_get_image(proc));
		__print_out("Thread ID\tStack\tPri\tState\tEntry\tName\n");
		__print_out("---------\t-----\t---\t-----\t-----\t----\n");

		while ( t )
		{
			__printp_out("\t%d", __thread_get_tid(t));
			__printp_out("\t0x%X", __thread_get_stack_size(t));
			__printp_out("\t%d", __thread_get_priority(t));
			__printp_out("\t%s", __ksh_thread_states[__thread_get_state(t)]);
			__printp_out("\t%x", __thread_get_entry_point(t));
			__printp_out("\t%s", __thread_get_name(t));
			__print_out("\n");

			if ( !__thread_it_t_next(tlist, &t))
			{
				t = NULL;
			}
		}
		__print_out("\n");

		__thread_it_t_delete(tlist);

		if ( !__process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	__process_list_it_t_delete(list);
}

static void __kshell_object_table(void)
{
	__process_list_it_t * list = NULL;
	__process_t * proc = NULL;

	list = __proc_list_procs();
	__process_list_it_t_get(list, &proc);

	while(proc)
	{
		__object_table_it_t * const it = __obj_iterator(__process_get_object_table(proc));

		__printp_out("Process:\t%s\n", __process_get_image(proc));
		__print_out("ObjNo.\tType\t\n");
		__print_out("------\t----\t\n");

		if (it)
		{
			__object_t * obj = NULL;

			__object_table_it_t_get(it, &obj);

			while( obj )
			{
				__object_type_t type = __obj_get_type(obj);

				__printp_out("%d\t", __obj_get_number(obj));
				__printp_out("%s\t", __ksh_object_types[type]);

				switch( type )
				{
					case UNKNOWN_OBJ:
					case OBJECT:
						break;

					case PROCESS_OBJ:
					{
						__object_process_t * const p = __obj_cast_process(obj);
						if (p)
						{
							__printp_out("PId:\t%d", __obj_process_pid(p));
						}
						else
						{
							__print_out("Invalid Object");
						}
					}
						break;
					case THREAD_OBJ:
					{
						__object_thread_t * const t = __obj_cast_thread(obj);
						if (t)
						{
							__thread_t * thread = __obj_get_thread(t);
							if (thread)
							{
								__printp_out("Tid:\t%d\t", __thread_get_tid(thread));
								__printp_out("PriInherit: %d", __thread_get_priority(thread));
							}
						}
						else
						{
							__print_out("Invalid Object");
						}
					}
						break;
					case PIPE_OBJ:
						/* TODO KSHELL dump for Pipe */
						break;
					case SEMAPHORE_OBJ:
					{
						__object_sema_t * const s = __obj_cast_semaphore(obj);
						if (s)
						{
							__printp_out("Count:\t%d\t", __obj_get_sema_count(s));
							__printp_out("Alloc:\t%d\t", __obj_get_sema_alloc(s));
							__printp_out("HighPri:%d", __obj_get_sema_highest_priority(s));
						}
						else
						{
							__print_out("Invalid Object");
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

				__print_out("\n");

				if ( !__object_table_it_t_next(it, &obj))
				{
					obj = NULL;
				}
			}

			__print_out("\n");

			__object_table_it_t_delete(it);
		}

		if ( !__process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	__process_list_it_t_delete(list);
}
#endif /* __KERNEL_SHELL */
