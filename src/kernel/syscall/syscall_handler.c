/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "syscall_handler.h"

#include "arch/tgt.h"
#include "arch/board_support.h"
#include "tinker_api_kernel_interface.h"
#include "kernel/kernel_initialise.h"
#include "kernel/kernel_main.h"
#include "kernel/process/process_list.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/objects/object.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/objects/obj_process.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/objects/obj_pipe.h"
#include "kernel/objects/obj_shared_mem.h"
#include "kernel/objects/obj_timer.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/time/time_manager.h"

#define MAX_SYSCALL_ARGS 7

typedef struct syscall_handler_t
{
	proc_list_t * process_list;
	registry_t * reg;
} syscall_handler_t;

static inline object_number_t syscall_get_thread_oid(const thread_t * const thread)
{
	return thread_get_object_no(thread);
}

static inline object_thread_t * syscall_get_thread_object(const thread_t * const thread)
{
	return (object_thread_t*)obj_get_object(process_get_object_table(thread_get_parent(thread)), syscall_get_thread_oid(thread));
}

static error_t syscall_delete_object(
		const object_number_t obj_no)
{
	return obj_remove_object(
			process_get_object_table(
					thread_get_parent(sch_get_current_thread())),
					obj_no);
}

static error_t syscall_get_sema(
		const thread_t * const thread,
		const object_number_t sema_no,
		object_sema_t ** sema)
{
	error_t ret = UNKNOWN_ERROR;

	const object_table_t * const table =
			process_get_object_table(thread_get_parent(sch_get_current_thread()));

	object_thread_t * const thread_obj = syscall_get_thread_object(thread);
	if (table && thread_obj)
	{
		object_t * const possible_sema = obj_get_object(table,sema_no);
		if (possible_sema)
		{
			object_sema_t * const sema_obj = obj_cast_semaphore(possible_sema);
			if (sema_obj)
			{
				*sema = sema_obj;
				ret = NO_ERROR;
			}
			else
			{
				ret = WRONG_OBJ_TYPE;
			}
		}
		else
		{
			ret = INVALID_OBJECT;
		}
	}
	else
	{
		ret = INVALID_OBJECT;
	}
	return ret;
}

syscall_handler_t * create_handler(
		mem_pool_info_t * const pool,
		proc_list_t * const proc_list,
		registry_t * const reg)
{
	syscall_handler_t * const sys = mem_alloc(pool, sizeof(syscall_handler_t));
	if (sys)
	{
		sys->process_list = proc_list;
		sys->reg = reg;
	}
	return sys;
}

void syscall_handle_system_call(
		syscall_handler_t * const handler,
		tgt_context_t * const context)
{
	syscall_function_t api = (syscall_function_t)tgt_get_syscall_param(context, 0);
	error_t ret = UNKNOWN_ERROR;
	uint32_t param[MAX_SYSCALL_ARGS];
	param[0] = tgt_get_syscall_param(context, 1);
	param[1] = tgt_get_syscall_param(context, 2);
	param[2] = tgt_get_syscall_param(context, 3);
	param[3] = tgt_get_syscall_param(context, 4);
	param[4] = tgt_get_syscall_param(context, 5);
	param[5] = tgt_get_syscall_param(context, 6);
	param[6] = tgt_get_syscall_param(context, 7);
	thread_t * const this_thread = sch_get_current_thread();

	/* This is accounting for things being passed on the
	 * stack which'll have a different base address as they'll be
	 * at some weird virtual address */
#if defined(ARCH_HAS_MMU)
	/* FIXME This is the bad code, fix it */
	for (uint8_t i = 0 ; i < MAX_SYSCALL_ARGS ; i++)
	{
		if (param[i] >= VIRTUAL_ADDRESS_SPACE)
		{
			param[i] = process_virt_to_real(thread_get_parent(this_thread), param[i]);
		}
	}
#endif

#if defined(SYSCALL_DEBUGGING)
	debug_print("Syscall: API %d\n", api);
#endif

	/*
	 * This could use a jump table but I think in this
	 * case this is a bit more readable.
	 */
	switch (api)
	{
		case SYSCALL_TEST:
			if (param[0] != SYSCALL_TEST_1
					|| param[1] != SYSCALL_TEST_2
					|| param[2] != SYSCALL_TEST_3
					|| param[3] != SYSCALL_TEST_4
					|| param[4] != SYSCALL_TEST_5
					|| param[5] != SYSCALL_TEST_6
					|| param[6] != SYSCALL_TEST_7)
			{
				kernel_panic();
			}
			ret = NO_ERROR;
			break;

		case SYSCALL_CREATE_PROCESS:
			{
				process_t * process;
				ret = proc_create_process(
						handler->process_list,
						(const char *)param[0],
						"main",
						(thread_entry_point*)(param[1]),
						(const uint8_t)param[2],
						(const tinker_meminfo_t* const)param[3],
						(uint32_t)param[4],
						(process_t **)&process);
				*((object_number_t*)param[5]) = process_get_oid(process);
			}
			break;

		case SYSCALL_CREATE_THREAD:
			{
				process_t * const process = thread_get_parent(this_thread);
				thread_t * thread = NULL;
				ret = proc_create_thread(
						process,
						(const char*)param[0],
						(thread_entry_point*)(param[1]),
						(const uint8_t)param[2],
						(const uint32_t)param[3],
						(uint32_t)param[4],
						NULL,
						(thread_t **)&thread);
				*((object_number_t*)param[5]) = thread_get_object_no(thread);
			}
			break;

		case SYSCALL_DEBUG:
		{
			const char * const msg = (const char * const)param[0];
			if (msg)
			{
				print_time();
				print_out(msg);
			}
			ret = NO_ERROR;
		}
			break;

		case SYSCALL_THREAD_PRIORITY:
			{
				object_number_t thread_no = (object_number_t)param[0];
				priority_t * priority = (priority_t*)param[1];
				if (thread_no && priority)
				{
					object_table_t * const table = process_get_object_table(thread_get_parent(this_thread));
					object_t * const obj = obj_get_object(table, thread_no);
					object_thread_t * thread_obj;
					if (obj)
					{
						thread_obj = obj_cast_thread(obj);
					}
					if (thread_obj)
					{
						ret = obj_get_thread_priority(thread_obj, priority);
					}
					else
					{
						ret = INVALID_OBJECT;
					}
				}
				else
				{
					ret = INVALID_OBJECT;
				}
			}
			break;

		case SYSCALL_THREAD_OBJECT:
			{
				const object_number_t objno = syscall_get_thread_oid(this_thread);
				*((object_number_t*)param[0]) = objno;
				ret = NO_ERROR;
			}
			break;

		case SYSCALL_EXIT_THREAD:
			{
				process_t * const parent = thread_get_parent(this_thread);
				const object_table_t * const table = process_get_object_table(parent);
				object_thread_t * const thread_obj
					= obj_cast_thread(obj_get_object(table, syscall_get_thread_oid(this_thread)));
				if (thread_obj)
				{
					object_process_t * const proc_obj =
							obj_cast_process(
								obj_get_object(table,
									process_get_oid(parent)));

					if (proc_obj)
					{
						ret = obj_process_thread_exit(proc_obj, thread_obj);
					}
				}
			}
			break;

		case SYSCALL_WAIT_THREAD:
			{
				thread_set_state(this_thread, THREAD_WAITING);
			}
			break;

		case SYSCALL_CREATE_SEMAPHORE:
			if (param[0])
			{
				if (this_thread)
				{
					process_t * const proc = thread_get_parent(this_thread);
					if (proc)
					{
						ret = obj_create_semaphore(
								handler->reg,
								proc,
								(object_number_t*)param[0],
								(char*)param[1],
								(const uint32_t)param[2]);
					}
					else
					{
						ret = PARAMETERS_INVALID;
					}
				}
				else
				{
					ret = PARAMETERS_INVALID;
				}

			}
			break;

		case SYSCALL_OPEN_SEMAPHORE:
			if (this_thread)
			{
				process_t * const proc = thread_get_parent(this_thread);
				if (proc)
				{
					ret = obj_open_semaphore(handler->reg, proc, (object_number_t*)param[0],(char*)param[1]);
				}
				else
				{
					ret = PARAMETERS_INVALID;
				}
			}
			else
			{
				ret = PARAMETERS_INVALID;
			}

			break;

		case SYSCALL_GET_SEMAPHORE:
		{
			object_thread_t * const thread_obj = syscall_get_thread_object(this_thread);
			object_sema_t * sema_obj = NULL;

			ret = syscall_get_sema(this_thread, (object_number_t)param[0], &sema_obj);
			if (ret == NO_ERROR && sema_obj && thread_obj)
			{
				ret = obj_get_semaphore( thread_obj, sema_obj);
			}
			else
			{
				ret = INVALID_OBJECT;
			}
			/* TODO check the return vector if the current thread isn't right */
			break;
		}
		case SYSCALL_RELEASE_SEMAPHORE:
		{
			object_thread_t * const thread_obj = syscall_get_thread_object(this_thread);
			object_sema_t * sema_obj = NULL;

			ret = syscall_get_sema(this_thread, (object_number_t)param[0], &sema_obj);
			if (ret == NO_ERROR && sema_obj && thread_obj)
			{
				ret = obj_release_semaphore( thread_obj, sema_obj);
			}
			break;
		}
		case SYSCALL_CLOSE_SEMAPHORE:
		{
			object_sema_t * sema_obj = NULL;

			ret = syscall_get_sema(this_thread, (object_number_t)param[0], &sema_obj);
			if (ret == NO_ERROR && sema_obj)
			{
				ret = object_delete_semaphore(sema_obj);
				if (ret == NO_ERROR)
				{
					ret = syscall_delete_object((object_number_t)param[0]);
				}
			}
			break;
		}
		case SYSCALL_CREATE_PIPE:
			ret = obj_create_pipe(
					handler->reg,
					thread_get_parent(this_thread),
					(object_number_t*)param[0],
					(const char*)param[1],
					(const tinker_pipe_direction_t)param[2],
					(const uint32_t)param[3],
					(const uint32_t)param[4]);
			break;
		case SYSCALL_DELETE_PIPE:
		{
			object_table_t * table = NULL;
			table = process_get_object_table(thread_get_parent(this_thread));
			object_pipe_t * const pipe = obj_cast_pipe(
					obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = obj_delete_pipe(pipe);
			if (ret == NO_ERROR)
			{
				ret = syscall_delete_object((object_number_t)param[0]);
			}
		}
			break;
		case SYSCALL_OPEN_PIPE:
			ret = obj_open_pipe(
					handler->reg,
					thread_get_parent(this_thread),
					syscall_get_thread_object(this_thread),
					(object_number_t*)param[0],
					(const char*)param[1],
					(const tinker_pipe_direction_t)param[2],
					(const uint32_t)param[3],
					(const uint32_t)param[4]);
			break;
		case SYSCALL_CLOSE_PIPE:
		{
			object_table_t * table = NULL;
			table = process_get_object_table(thread_get_parent(this_thread));
			object_pipe_t * const pipe = obj_cast_pipe(
					obj_get_object(
							table,
							(object_number_t)param[0]));
			// FIXME This will release memory but isn't right,
			// close needs to remove the item from the senders list etc
			ret = obj_delete_pipe(pipe);
			if (ret == NO_ERROR)
			{
				ret = syscall_delete_object((object_number_t)param[0]);
			}
		}
			break;
		case SYSCALL_SEND_MESSAGE:
		{
			object_table_t * const table = process_get_object_table(thread_get_parent(this_thread));
			object_pipe_t * const pipe = obj_cast_pipe(
					obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = obj_pipe_send_message(
					pipe,
					syscall_get_thread_object(this_thread),
					(tinker_pipe_send_kind_t)param[1],
					(void*)param[2],
					(const uint32_t)param[3],
					(const bool_t)param[4]);
		}
			break;
		case SYSCALL_RECEIVE_MESSAGE:
		{
			object_table_t * const table = process_get_object_table(thread_get_parent(this_thread));
			object_pipe_t * const pipe = obj_cast_pipe(
					obj_get_object(
							table,
							(object_number_t)param[0]));

			uint8_t ** msg = (uint8_t**)param[1];
			uint32_t ** msg_size = (uint32_t**)param[2];
			ret = obj_pipe_receive_message(
					pipe,
					syscall_get_thread_object(this_thread),
					(void**)msg,
					msg_size,
					(const bool_t)param[3]);

#if defined (ARCH_HAS_MMU)
			// FIXME this shouldn't be here - it should be in the pipe object code
			process_t * const proc = thread_get_parent(this_thread);
			const uint32_t pool_start = mem_get_start_addr(
					process_get_mem_pool(proc));
			if (!process_is_kernel(proc))
			{
				*msg += VIRTUAL_ADDRESS_SPACE;
				*msg -= pool_start;
				msg_size += VIRTUAL_ADDRESS_SPACE;
				msg_size -= pool_start;
			}
#endif
		}
			break;
		case SYSCALL_RECEIVED_MESSAGE:
		{
			object_table_t * const table = process_get_object_table(thread_get_parent(this_thread));
			object_pipe_t * const pipe = obj_cast_pipe(
					obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = obj_pipe_received_message(pipe);
		}
			break;

		case SYSCALL_CREATE_SHM:
			ret = obj_create_shm(
					handler->reg,
					thread_get_parent(this_thread),
					(object_number_t*)param[0],
					(char*)param[1],
					(uint32_t)param[2],
					(void**)param[3]);
			break;
		case SYSCALL_OPEN_SHM:
			ret = obj_open_shm(
					handler->reg,
					thread_get_parent(this_thread),
					(object_number_t*)param[0],
					(char*)param[1],
					(uint32_t)param[2],
					(void**)param[3]);
			break;

		case SYSCALL_DESTROY_SHM:
		{
			object_table_t * const table =
					process_get_object_table(
							thread_get_parent(this_thread));
			object_shm_t * const shm = obj_cast_shm(
					(object_t *)obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = obj_delete_shm(shm);
			if (ret == NO_ERROR)
			{
				ret = syscall_delete_object((object_number_t)param[0]);
			}
			break;
		}
		case SYSCALL_CREATE_TIMER:
			ret = obj_create_timer(
					thread_get_parent(this_thread),
					(object_number_t*)param[0],
					(const priority_t)param[1],
					(const uint32_t)param[2],
					(const uint32_t)param[3],
					(tinker_timer_callback_t*)param[4],
					(void*)param[5]);
			break;
		case SYSCALL_CANCEL_TIMER:
		{
			object_table_t * const table =
					process_get_object_table(
							thread_get_parent(this_thread));
			object_timer_t * const timer = obj_cast_timer(
					(object_t *)obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = obj_cancel_timer(timer);
			break;
		}
		case SYSCALL_DELETE_TIMER:
		{
			object_table_t * const table =
					process_get_object_table(
							thread_get_parent(this_thread));
			object_timer_t * const timer = obj_cast_timer(
					(object_t *)obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = obj_delete_timer(timer);
			if (ret == NO_ERROR)
			{
				ret = syscall_delete_object((object_number_t)param[0]);
			}
			break;
		}
		case SYSCALL_GET_TIME:
			if (param[0])
			{
				time_get_system_time(((tinker_time_t*)param[0]));
				ret = NO_ERROR;
			}
			else
			{
				ret = PARAMETERS_NULL;
			}
			break;
		case SYSCALL_SLEEP:
		{
			const tinker_time_t * const duration = (const tinker_time_t*)param[0];
			object_thread_t * const thread_obj =syscall_get_thread_object(this_thread);
			ret = obj_thread_sleep(thread_obj, duration);
		}
		break;
		case SYSCALL_WFI:
			tgt_wait_for_interrupt();
			break;
		case SYSCALL_LOAD_THREAD:
			/* uses the current thread from the scheduler */
			tgt_prepare_context(context, this_thread, NULL);
			break;

		default:
			/* setup the error_number and add associated error number utilities */
			ret = SYSCALL_UNKNOWN;
			break;
	}

	/* This will over-ride the result of a system-call
	 * if the exception occurs just after the system call has been made. */
	bool_t scheduled = false;
	if (api != SYSCALL_LOAD_THREAD && api != SYSCALL_EXIT_THREAD)
	{
		tgt_set_syscall_return(context, ret);
	}
	else
	{
		scheduled = true;
		bsp_enable_schedule_timer();
	}

	/* If the thread has been un-scheduled we need to switch process */
	thread_state_t state;
	if (api == SYSCALL_EXIT_THREAD)
	{
		state = THREAD_DEAD;
	}
	else
	{
		state = thread_get_state(this_thread);
	}
	if ( (state != THREAD_SYSTEM) &&
		 (state != THREAD_RUNNING) )
	{
		/* save the existing data - i.e. the return & run the scheduler */
		sch_set_context_for_next_thread(context, state);
		if (!scheduled)
		{
			bsp_enable_schedule_timer();
		}
	}
#if defined(SYSCALL_DEBUGGING)
	debug_print("Syscall: API %d RET %d\n", api, ret);
#endif
}
