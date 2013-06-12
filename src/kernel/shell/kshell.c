/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kshell.h"
#if defined(__KERNEL_SHELL)
#include "api/sos_api_types.h"
#include "kernel/debug/debug_print.h"
#include "kernel/debug/debug_read.h"
#include "kernel/process/process_manager.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_memset.h"
#include "kernel/objects/object_table.h"
#include "kernel/utils/collections/hashed_map.h"
#include "kernel/utils/collections/hashed_map_iterator.h"

#define MAX_LINE_INPUT 256

static char __ksh_input_buffer[MAX_LINE_INPUT];

static uint8_t __ksh_input_pointer;

static bool __kshell_strcmp(const char * a, const char * b);

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

HASH_MAP_TYPE_T(object_map_t, uint32_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)
HASH_MAP_TYPE_ITERATOR_SPEC(static, obj_tbl_it_t, object_map_t, __object_t*)
HASH_MAP_TYPE_ITERATOR_BODY(static, obj_tbl_it_t, object_map_t, uint32_t, __object_t*, __MAX_OBJECT_TABLE_SIZE)

void __kshell_start(void)
{
	__ksh_input_pointer = 0;
	while(1)
	{
		__ksh_input_buffer[__ksh_input_pointer] = __debug_read();
		if ( __ksh_input_buffer[__ksh_input_pointer] == '\r'
				|| __ksh_input_buffer[__ksh_input_pointer] == '\n' )
		{
			__ksh_input_buffer[__ksh_input_pointer] = '\0';
			if ( __ksh_input_pointer )
			{
				__kshell_execute_command(__ksh_input_buffer);
				__ksh_input_pointer = 0;
				__util_memset(__ksh_input_buffer, 0, MAX_LINE_INPUT);
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
	if ( __kshell_strcmp(command, "procs") )
	{
		__kshell_process_list();
	}
	else if ( __kshell_strcmp(command, "tasks") )
	{
		__kshell_task_list();
	}
	else if ( __kshell_strcmp(command, "objects"))
	{
		__kshell_object_table();
	}
}

static bool __kshell_strcmp(const char * a, const char * b)
{
	const uint32_t l = __util_strlen(a, 65535);

	if ( l != __util_strlen(b, 65535) )
	{
		return false;
	}

	uint32_t p;
	bool ok = true;

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
	process_list_it_t * list = NULL;
	__process_t * proc = NULL;

	list = __proc_get_new_process_iterator();
	process_list_it_t_get(list, &proc);

	__debug_print("ProcessId\tThreads\tName\n");
	__debug_print("---------\t-------\t----\n");

	while( proc )
	{
		__debug_print("\t%d", proc->process_id);
		__debug_print("\t%d", proc->thread_count);
		__debug_print("\t%s\n", proc->image);
		if ( !process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
}

static void __kshell_task_list(void)
{
	process_list_it_t * list = NULL;
	__process_t * proc = NULL;

	list = __proc_get_new_process_iterator();
	process_list_it_t_get(list, &proc);

	while( proc )
	{
		thread_list_it_t * tlist = NULL;
		__thread_t  * t = NULL;

		tlist = thread_list_it_t_create(proc->threads);
		thread_list_it_t_get(tlist, &t);

		__debug_print("Process:\t%s\n", proc->image);
		__debug_print("Thread ID\tStack\tPri\tState\tEntry\tName\n");
		__debug_print("---------\t-----\t---\t-----\t-----\t----\n");

		while ( t )
		{
			__debug_print("\t%d", t->thread_id);
			__debug_print("\t0x%X", t->stack_size);
			__debug_print("\t%d", t->priority);
			__debug_print("\t%s", __ksh_thread_states[t->state]);
			__debug_print("\t%x", t->entry_point);
			__debug_print("\t%s", t->name);
			__debug_print("\n");

			if ( !thread_list_it_t_next(tlist, &t))
			{
				t = NULL;
			}
		}
		__debug_print("\n");

		thread_list_it_t_delete(tlist);

		if ( !process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
}

static void __kshell_object_table(void)
{
	process_list_it_t * list = NULL;
	__process_t * proc = NULL;

	list = __proc_get_new_process_iterator();
	process_list_it_t_get(list, &proc);

	while( proc )
	{
		__object_table_t * tbl = &proc->object_table;

		__debug_print("Process:\t%s\n", proc->image);
		__debug_print("ObjNo.\tInit\tAlloc\tType\t\n");
		__debug_print("------\t----\t-----\t----\t\n");

		if ( tbl )
		{
			__object_t * obj = NULL;

			obj_tbl_it_t * it = obj_tbl_it_t_create(tbl->the_map);

			obj_tbl_it_t_get(it, &obj);

			while( obj )
			{
				__debug_print("%d\t", obj->object_number);
				__debug_print("%x\t", obj->initialised);
				__debug_print("%x\t", obj->allocated);
				__debug_print("%s\t", __ksh_object_types[obj->type]);

				switch( obj->type )
				{
					case UNKNOWN_OBJ:
					case OBJECT:
						break;

					case PROCESS_OBJ:
						__debug_print("PId:\t%d", obj->specifics.process.pid);
						break;
					case THREAD_OBJ:
						__debug_print("Tid:\t%d\t", obj->specifics.thread.tid);
						__debug_print("PriInherit: %d", obj->specifics.thread.priority_inheritance);
						break;
					case PIPE_OBJ:
						/* TODO KSHELL dump for Pipe */
						break;
					case SEMAPHORE_OBJ:
						__debug_print("Count:\t%d\t", obj->specifics.semaphore.sem_count);
						__debug_print("Alloc:\t%d\t", obj->specifics.semaphore.sem_alloc);
						__debug_print("HighPri:%d", obj->specifics.semaphore.highest_priority);
						break;
					case SHARED_MEMORY_OBJ:
						/* TODO KSHELL dump for Shared Memory */
						break;
					case CLOCK_OBJ:
						/* TODO KSHELL dump for Clock */
						break;
					case TIMER_OBJ:
						/* TODO KSHELL dump for Timer */
						break;
				}

				__debug_print("\n");

				if ( !obj_tbl_it_t_next(it, &obj))
				{
					obj = NULL;
				}
			}

			__debug_print("\n");

			obj_tbl_it_t_delete(it);
		}

		if ( !process_list_it_t_next(list, &proc) )
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
}
#endif /* __KERNEL_SHELL */
