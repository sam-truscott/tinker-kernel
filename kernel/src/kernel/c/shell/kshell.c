/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "shell/kshell.h"
#if defined(KERNEL_SHELL)
#include "tinker_api_types.h"
#include "tinker_api_pipe.h"
#include "tgt.h"
#include "console/print_out.h"
#include "process/process_list.h"
#include "utils/util_strlen.h"
#include "utils/util_memset.h"
#include "objects/object_table.h"
#include "objects/obj_semaphore.h"
#include "objects/obj_process.h"
#include "objects/obj_thread.h"
#include "objects/obj_pipe.h"
#include "process/thread.h"
#include "utils/collections/hashed_map.h"
#include "utils/collections/hashed_map_iterator.h"

#define MAX_LINE_INPUT 256

typedef struct kshell_t
{
	proc_list_t * proc_list;
	uint16_t ksh_input_pointer;
	char ksh_input_buffer[MAX_LINE_INPUT];
} kshell_t;

static kshell_t * kshell;

static char kshell_dev_name[MAX_SHARED_OBJECT_NAME_LENGTH];

static bool_t kshell_strcmp(const char * a, const char * b);

static void kshell_execute_command(const char* command);

static void kshell_process_list(void);

static void kshell_task_list(void);

static void kshell_object_table(void);

static void kshell_memory_info(void);

static const char ksh_thread_states[10][8] =
{
		"ERROR  \0",
		"NCREATE\0",
		"IDLE   \0",
		"READY  \0",
		"RUNNING\0",
		"SYSTEM \0",
		"PAUSED \0",
		"WAITING\0",
		"TERMATE\0",
		"DEAD   \0"
};

static const char ksh_object_types[9][8] =
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

static const char ksh_pipe_dir[4][13] =
{
		"UNKN\0",
		"BOTH\0",
		"TX  \0",
		"RX  \0"
};

static const char ksh_mem_type[2][4] =
{
		"DEV\0",
		"MEM\0"
};

static const char ksh_mem_priv[4][5] =
{
		"NONE\0",
		"USER\0",
		"KERN\0",
		"ALL \0"
};

static const char ksh_mem_acc[3][4] =
{
		"N/A\0",
		"RO \0",
		"RW \0"
};

void kshell_setup(
		mem_pool_info_t * const pool,
		proc_list_t * const proc_list)
{
	kshell = mem_alloc(pool, sizeof(kshell_t));
	if (kshell)
	{
		kshell->proc_list = proc_list;
	}
}

void kshell_set_input_device(const char * const dev)
{
	util_memset(kshell_dev_name, 0, MAX_SHARED_OBJECT_NAME_LENGTH);
	util_memcpy(kshell_dev_name, dev, util_strlen(dev, MAX_SHARED_OBJECT_NAME_LENGTH));
}

void kshell_start(void)
{
	kshell->ksh_input_pointer = 0;
	bool_t running = true;

	print_out("Tinker Shell: Starting...\n");
	print_out("Commands: procs, tasks, objects, mem\n");

	tinker_pipe_t pipe;
	return_t input_result = tinker_open_pipe(
			&pipe,
			kshell_dev_name,
			PIPE_RECEIVE,
			1,
			MAX_LINE_INPUT);

	if (input_result != NO_ERROR)
	{
		printp_out("KSHELL failed to open input pipe, error was %d\n", input_result);
		return;
	}

	util_memset(kshell->ksh_input_buffer, 0, MAX_LINE_INPUT);

	print_out("Tinker Shell: Ready\n");

	// kshell_process_list();
	// kshell_task_list();
	// kshell_memory_info();
	// kshell_object_table();
	while (running)
	{
		char received[2];
		util_memset(received, 0, sizeof(received));
		uint32_t bytesReceived = 0;
		if (is_debug_enabled(SHELL))
		{
			print_out("KSHELL Rx\n");
		}
		return_t read_status = tinker_receive_message(pipe, (void*)received, &bytesReceived, MAX_LINE_INPUT, true);
		if (is_debug_enabled(SHELL))
		{
			printp_out("KSHELL status = %d, got %d bytes: %d\n", read_status, bytesReceived, received[0]);
		}
		if (read_status == NO_ERROR)
		{
			uint16_t p = 0;
			while (p < bytesReceived)
			{
				kshell->ksh_input_buffer[kshell->ksh_input_pointer++] = received[p++];
			}
			kshell->ksh_input_pointer--;
			if (kshell->ksh_input_buffer[kshell->ksh_input_pointer] == '\r'
					|| kshell->ksh_input_buffer[kshell->ksh_input_pointer] == '\n')
			{
				kshell->ksh_input_buffer[kshell->ksh_input_pointer] = '\0';
				if (kshell->ksh_input_pointer)
				{
					if (kshell_strcmp(kshell->ksh_input_buffer, "exit"))
					{
						running = false;
					}
					else
					{
						kshell_execute_command(kshell->ksh_input_buffer);
						kshell->ksh_input_pointer = 0;
						util_memset(kshell->ksh_input_buffer, 0, MAX_LINE_INPUT);
						print_out("> ");
					}
				}
			}
			else
			{
				kshell->ksh_input_pointer++;
			}
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
	else if (kshell_strcmp(command, "objects"))
	{
		kshell_object_table();
	}
	else if (kshell_strcmp(command, "mem"))
	{
		kshell_memory_info();
	}
	else
	{
		printp_out("%s?\n", command);
	}
}

static bool_t kshell_strcmp(const char * a, const char * b)
{
	const uint32_t length = util_strlen(a, MAX_LINE_INPUT);

	if (length != util_strlen(b, MAX_LINE_INPUT))
	{
		return false;
	}

	uint32_t p;
	bool_t ok = true;

	for (p = 0 ; p < length && p < MAX_LINE_INPUT && ok ; p++)
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

	list = proc_list_procs(kshell->proc_list);
	process_list_it_t_get(list, &proc);

	print_out("Proc. Id\tThreads \tName\n");
	print_out("--------\t--------\t----\n");

	while (proc)
	{
		printp_out("%8d", process_get_pid(proc));
		printp_out("\t%8d", process_get_thread_count(proc));
		printp_out("\t%s\n", process_get_image(proc));
		if (!process_list_it_t_next(list, &proc))
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);
	print_out("Complete\n");
}

static void kshell_task_list(void)
{
	process_list_it_t * list = NULL;
	process_t * proc = NULL;

	list = proc_list_procs(kshell->proc_list);
	process_list_it_t_get(list, &proc);

	while (proc)
	{
		thread_it_t * tlist = process_iterator(proc);
		thread_t  * t = NULL;

		thread_it_t_get(tlist, &t);

		printp_out("Process:\t%s\n", process_get_image(proc));
		print_out("ThreadID\tStack Sz  \tStack Pt  \tPri\tEntry   \tState\tPc\t\tName\n");
		print_out("--------\t----------\t----------\t---\t--------\t-----\t--\t\t----\n");

		while (t)
		{
			printp_out("%8d", thread_get_tid(t));
			printp_out("\t0x%8X", thread_get_stack_size(t));
			const tgt_context_t * context = thread_get_context(t);
			printp_out("\t0x%8X", tgt_get_context_stack_pointer(context));
			printp_out("\t%3d", thread_get_priority(t));
			printp_out("\t%8x", thread_get_entry_point(t));
			printp_out("\t%s", ksh_thread_states[thread_get_state(t)]);
			printp_out("\t%8x", tgt_get_pc(context));
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
	print_out("Complete\n");
}

static void kshell_object_table(void)
{
	process_list_it_t * const list = proc_list_procs(kshell->proc_list);
	process_t * proc = NULL;
	process_list_it_t_get(list, &proc);

	while (proc)
	{
		object_table_it_t * const it = obj_iterator(process_get_object_table(proc));

		printp_out("Process:\t%s\n", process_get_image(proc));
		print_out("Obj No. \tType\t\n");
		print_out("--------\t----\t\n");

		if (it)
		{
			object_t * obj = NULL;
			object_table_it_t_get(it, &obj);

			while (obj)
			{
				const object_type_t type = obj_get_type(obj);

				printp_out("%8d\t", obj_get_number(obj));
				printp_out("%s\t", ksh_object_types[type]);

				switch (type)
				{
					default:
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
					{
						const object_pipe_t * const p = obj_cast_pipe(obj);
						if (p)
						{
							printp_out("Name:\t%s, ", obj_pipe_get_name(p));
							tinker_pipe_direction_t const dir = obj_pipe_get_direction(p);
							printp_out("%s, ", ksh_pipe_dir[obj_pipe_get_direction(p)]);
							switch (dir)
							{
								case PIPE_SEND:
									break;
								case PIPE_SEND_RECEIVE:
								case PIPE_RECEIVE:
									printp_out("Total: %d, ", obj_pipe_get_total_messages(p));
									printp_out("Free: %d, ", obj_pipe_get_free_messages(p));
									printp_out("Sz: %d, ", obj_pipe_get_msg_size(p));
									printp_out("Rd Pos: %d, ", obj_pipe_get_read_msg_pos(p));
									printp_out("Wr Pos: %d", obj_pipe_get_write_msg_pos(p));
									break;
								default:
									break;
							}
						}
						else
						{
							print_out("Invalid Object");
						}
					}
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

				if (!object_table_it_t_next(it, &obj))
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
	print_out("Complete\n");
}

static void kshell_memory_info(void)
{
	print_out("mem:\n\n");
	process_list_it_t * list = proc_list_procs(kshell->proc_list);
	process_t * proc = NULL;
	process_list_it_t_get(list, &proc);

	print_out("Proc\tPool Sz    Pool Usd   P.Pool Sz  P.Pool Usd\n");
	print_out("----\t---------- ---------- ---------- ----------\n");

	while (proc)
	{
		printp_out("%s\t", process_get_image(proc));
		mem_pool_info_t * const pool = process_get_user_mem_pool(proc);
		mem_pool_info_t * const prv_pool = process_get_mem_pool(proc);
		printp_out("0x%8x 0x%8x 0x%8x 0x%8x\n",
				mem_get_alloc_size(pool),
				mem_get_allocd_size(pool),
				mem_get_alloc_size(prv_pool),
				mem_get_allocd_size(prv_pool));

		if (!process_list_it_t_next(list, &proc))
		{
			proc = NULL;
		}
	}

	process_list_it_t_delete(list);

	print_out("\nMemory mappings:\n");

	print_out("Real       Virt       Size       Typ Priv Access Name\n");
	print_out("---------- ---------- ---------- --- ---- ------ ----\n");

	list = proc_list_procs(kshell->proc_list);
	process_list_it_t_get(list, &proc);
	while (proc)
	{
		printp_out("%s\n", process_get_image(proc));
		const mem_section_t * sec = process_get_first_section(proc);
		while (sec)
		{
			printp_out("0x%8x 0x%8x 0x%8x %s %s %s     %s\n",
					mem_sec_get_real_addr(sec),
					mem_sec_get_virt_addr(sec),
					mem_sec_get_size(sec),
					ksh_mem_type[mem_sec_get_mem_type(sec)],
					ksh_mem_priv[mem_sec_get_priv(sec)],
					ksh_mem_acc[mem_sec_get_access(sec)],
					mem_sec_get_name(sec));
			sec = mem_sec_get_next(sec);
		}
		if (!process_list_it_t_next(list, &proc))
		{
			proc = NULL;
		}
	}
	process_list_it_t_delete(list);
	print_out("Complete\n");
}
#endif /* KERNEL_SHELL */
