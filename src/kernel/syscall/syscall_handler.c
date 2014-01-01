/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "syscall_handler.h"

#include "arch/tgt.h"
#include "arch/board_support.h"
#include "sos_api_kernel_interface.h"
#include "kernel/kernel_initialise.h"
#include "kernel/kernel_main.h"
#include "kernel/process/process_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/objects/object.h"
#include "kernel/objects/obj_semaphore.h"
#include "kernel/objects/obj_process.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/obj_thread.h"
#include "kernel/objects/obj_pipe.h"
#include "kernel/objects/obj_shared_mem.h"
#include "kernel/utils/util_memcpy.h"

static inline object_number_t __syscall_get_thread_oid(void)
{
	return __thread_get_object_no(__sch_get_current_thread());
}

static inline __object_thread_t * __syscall_get_thread_object(void)
{
	const __process_t * const proc =  __thread_get_parent(__sch_get_current_thread());
	const __object_table_t * const table = __process_get_object_table(proc);
	return (__object_thread_t*)__obj_get_object(table, __syscall_get_thread_oid());
}

static error_t __syscall_delete_object(
		const object_number_t obj_no)
{
	return __obj_remove_object(
			__process_get_object_table(
					__thread_get_parent(__sch_get_current_thread())),
					obj_no);
}

static error_t __syscall_get_sema(
		const object_number_t sema_no,
		__object_sema_t ** sema)
{
	error_t ret = UNKNOWN_ERROR;

	const __object_table_t * const table =
			__process_get_object_table(__thread_get_parent(__sch_get_current_thread()));

	__object_thread_t * const thread_obj = __syscall_get_thread_object();
	if (table && thread_obj)
	{
		__object_t * const possible_sema = __obj_get_object(table,sema_no);
		if (possible_sema)
		{
			__object_sema_t * const sema_obj = __obj_cast_semaphore(possible_sema);
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

void __syscall_handle_system_call(__tgt_context_t * const context)
{
	__syscall_function_t api = (__syscall_function_t)__tgt_get_syscall_param(context, 0);
	error_t ret = UNKNOWN_ERROR;
	/* FIXME hard coded 7 */
	uint32_t param[7];
	param[0] = __tgt_get_syscall_param(context, 1);
	param[1] = __tgt_get_syscall_param(context, 2);
	param[2] = __tgt_get_syscall_param(context, 3);
	param[3] = __tgt_get_syscall_param(context, 4);
	param[4] = __tgt_get_syscall_param(context, 5);
	param[5] = __tgt_get_syscall_param(context, 6);
	param[6] = __tgt_get_syscall_param(context, 7);
	__thread_t * this_thread = __sch_get_current_thread();

	/* TODO FIXME This is accounting for things being passed on the
	 * stack which'll have a different base address as they'll be
	 * at some weird virtual address */
	for (uint8_t i = 0 ; i < 7 ; i++)
	{
		if (param[i] >= VIRTUAL_ADDRESS_SPACE)
		{
			param[i] = __process_virt_to_real(__thread_get_parent(this_thread), param[i]);
		}
	}

	/*
	 * This could use a jump table but I think in this
	 * case this is a bit more readable.
	 */
	switch (api)
	{
		case SYSCALL_CREATE_PROCESS:
			{
				__process_t * process;
				ret = __proc_create_process(
						(const char *)param[0],
						"main",
						(thread_entry_point*)(param[1]),
						(const uint8_t)param[2],
						(const uint32_t)param[3],
						(const uint32_t)param[4],
						(uint32_t)param[5],
						(__process_t **)&process);
				*((object_number_t*)param[6]) = __process_get_oid(process);
			}
			break;

		case SYSCALL_CREATE_THREAD:
			{
				__process_t * process;
				__thread_t * thread;

				process = __thread_get_parent(this_thread);

				ret = __proc_create_thread(
						process,
						(const char*)param[0],
						(thread_entry_point*)(param[1]),
						(const uint8_t)param[2],
						(const uint32_t)param[3],
						(uint32_t)param[4],
						NULL,
						(__thread_t **)&thread);
				*((object_number_t*)param[5]) = __thread_get_object_no(thread);
			}
			break;

		case SYSCALL_DEBUG:
		{
			const char * const msg = (const char * const)param[0];
			if (msg)
			{
				__print_out(msg);
			}
		}
			break;

		case SYSCALL_THREAD_PRIORITY:
			{
				object_number_t thread_no = (object_number_t)param[0];
				priority_t * priority = (priority_t*)param[1];
				if ( thread_no && priority)
				{
					__object_table_t * table = NULL;
					__object_t * obj = NULL;
					__object_thread_t * thread_obj = NULL;

					table = __process_get_object_table(__thread_get_parent(__sch_get_current_thread()));

					obj = __obj_get_object(table, thread_no);
					if (obj)
					{
						thread_obj = __obj_cast_thread(obj);
					}
					if (thread_obj)
					{
						ret = __obj_get_thread_priority(thread_obj, priority);
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
				const object_number_t objno = __syscall_get_thread_oid();
				*((object_number_t*)param[0]) = objno;
				ret = NO_ERROR;
			}
			break;

		case SYSCALL_EXIT_THREAD:
			{
				__object_thread_t * const thread_obj =__syscall_get_thread_object();
				if ( thread_obj )
				{
					const __object_table_t * const table =
							__process_get_object_table(__thread_get_parent(this_thread));

					__object_process_t * const proc_obj =
							__obj_cast_process(
								__obj_get_object(table,
									__obj_thread_get_proc_oid(thread_obj)));

					if (proc_obj)
					{
						ret = __obj_process_thread_exit(proc_obj, thread_obj);
					}
				}
			}
			break;

		case SYSCALL_CREATE_SEMAPHORE:
			if (param[0])
			{
				if (this_thread)
				{
					__process_t * const proc = __thread_get_parent(this_thread);
					if (proc)
					{
						ret = __obj_create_semaphore(
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
				__process_t * const proc = __thread_get_parent(this_thread);
				if (proc)
				{
					ret = __obj_open_semaphore(proc, (object_number_t*)param[0],(char*)param[1]);
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
			__object_thread_t * const thread_obj = __syscall_get_thread_object();
			__object_sema_t * sema_obj;

			ret = __syscall_get_sema((object_number_t)param[0], &sema_obj);

			if (ret == NO_ERROR && sema_obj && thread_obj)
			{
				ret = __obj_get_semaphore( thread_obj, sema_obj);
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
			__object_thread_t * const thread_obj = __syscall_get_thread_object();
			__object_sema_t * sema_obj = NULL;

			ret = __syscall_get_sema((object_number_t)param[0], &sema_obj);

			if (ret == NO_ERROR && sema_obj && thread_obj)
			{
				ret = __obj_release_semaphore( thread_obj, sema_obj);
			}
			break;
		}
		case SYSCALL_CLOSE_SEMAPHORE:
		{
			__object_sema_t * sema_obj = NULL;

			ret = __syscall_get_sema((object_number_t)param[0], &sema_obj);
			if (ret == NO_ERROR && sema_obj)
			{
				ret = __object_delete_semaphore(sema_obj);
				if (ret == NO_ERROR)
				{
					ret = __syscall_delete_object((object_number_t)param[0]);
				}
			}
			break;
		}
		case SYSCALL_CREATE_PIPE:
			ret = __obj_create_pipe(
					__thread_get_parent(this_thread),
					(object_number_t*)param[0],
					(const char*)param[1],
					(const sos_pipe_direction_t)param[2],
					(const uint32_t)param[3],
					(const uint32_t)param[4]);
			break;
		case SYSCALL_DELETE_PIPE:
		{
			__object_table_t * table = NULL;
			table = __process_get_object_table(__thread_get_parent(__sch_get_current_thread()));
			__object_pipe_t * const pipe = __obj_cast_pipe(
					__obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = __obj_delete_pipe(pipe);
			if (ret == NO_ERROR)
			{
				ret = __syscall_delete_object((object_number_t)param[0]);
			}
		}
			break;
		case SYSCALL_OPEN_PIPE:
			ret = __object_open_pipe(
					__thread_get_parent(this_thread),
					__syscall_get_thread_object(),
					(object_number_t*)param[0],
					(const char*)param[1],
					(const sos_pipe_direction_t)param[2],
					(const uint32_t)param[3],
					(const uint32_t)param[4]);
			break;
		case SYSCALL_CLOSE_PIPE:
		{
			__object_table_t * table = NULL;
			table = __process_get_object_table(__thread_get_parent(__sch_get_current_thread()));
			__object_pipe_t * const pipe = __obj_cast_pipe(
					__obj_get_object(
							table,
							(object_number_t)param[0]));
			// FIXME This will release memory but isn't right,
			// close needs to remove the item from the senders list etc
			ret = __obj_delete_pipe(pipe);
			if (ret == NO_ERROR)
			{
				ret = __syscall_delete_object((object_number_t)param[0]);
			}
		}
			break;
		case SYSCALL_SEND_MESSAGE:
		{
			__object_table_t * table = NULL;
			table = __process_get_object_table(__thread_get_parent(__sch_get_current_thread()));
			__object_pipe_t * const pipe = __obj_cast_pipe(
					__obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = __obj_pipe_send_message(
					pipe,
					__syscall_get_thread_object(),
					(sos_pipe_send_kind_t)param[1],
					(void*)param[2],
					(const uint32_t)param[3],
					(const bool_t)param[4]);
		}
			break;
		case SYSCALL_RECEIVE_MESSAGE:
		{
			__object_table_t * table = NULL;
			table = __process_get_object_table(__thread_get_parent(this_thread));
			__object_pipe_t * const pipe = __obj_cast_pipe(
					__obj_get_object(
							table,
							(object_number_t)param[0]));

			uint8_t ** msg = (uint8_t**)param[1];
			uint32_t * msg_size = (uint32_t*)param[2];
			ret = __obj_pipe_receive_message(
					pipe,
					__syscall_get_thread_object(),
					(void**)msg,
					msg_size,
					(const bool_t)param[3]);

			const uint32_t pool_start = __mem_get_start_addr(
					__process_get_mem_pool(
							__thread_get_parent(this_thread)));
			*msg += VIRTUAL_ADDRESS_SPACE;
			*msg -= pool_start;
			msg_size += VIRTUAL_ADDRESS_SPACE;
			msg_size -= pool_start;
		}
			break;
		case SYSCALL_RECEIVED_MESSAGE:
		{
			__object_table_t * table = NULL;
			table = __process_get_object_table(__thread_get_parent(__sch_get_current_thread()));
			__object_pipe_t * const pipe = __obj_cast_pipe(
					__obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = __obj_pipe_received_message(pipe);
		}
			break;

		case SYSCALL_CREATE_SHM:
			ret = __obj_create_shm(
					__thread_get_parent(__sch_get_current_thread()),
					(object_number_t*)param[0],
					(char*)param[1],
					(uint32_t)param[2],
					(void**)param[3]);
			break;
		case SYSCALL_OPEN_SHM:
			ret = __obj_open_shm(
					__thread_get_parent(__sch_get_current_thread()),
					(object_number_t*)param[0],
					(char*)param[1],
					(uint32_t)param[2],
					(void**)param[3]);
			break;

		case SYSCALL_DESTROY_SHM:
		{
			__object_table_t * const table =
					__process_get_object_table(
							__thread_get_parent(__sch_get_current_thread()));
			__object_shm_t * const shm = __obj_cast_shm(
					(__object_t *)__obj_get_object(
							table,
							(object_number_t)param[0]));
			ret = __obj_delete_shm(shm);
			if (ret == NO_ERROR)
			{
				ret = __syscall_delete_object((object_number_t)param[0]);
			}
		}
			break;

		case SYSCALL_LOAD_THREAD:
			__tgt_prepare_context(context, this_thread);
			break;

		default:
			/* setup the error_number and add associated error number utilities */
			ret = SYSCALL_UNKNOWN;
			break;
	}

	/* This will over-ride the result of a system-call
	 * if the exception occurs just after the system call has been made. */
	if ( api != SYSCALL_LOAD_THREAD )
	{
		__tgt_set_syscall_return(context, ret);
	}
	else
	{
		__bsp_enable_schedule_timer();
	}

	/*
	 * If the thread has been un-scheduled we need to switch process
	 */
	const __thread_state_t state = __thread_get_state(this_thread);
	if ( (state != thread_system) &&
		 (state != thread_running) )
	{
		/* save the existing data - i.e. the return & run the scheduler */
		__sch_set_context_for_next_thread(context);
	}
}
