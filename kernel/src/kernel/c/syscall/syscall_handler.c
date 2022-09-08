/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "syscall/syscall_handler.h"

#include "tgt.h"
#include "board_support.h"
#include "tinker_api_kernel_interface.h"
#include "kernel_main.h"
#include "process/process_list.h"
#include "scheduler/scheduler.h"
#include "objects/object.h"
#include "objects/obj_semaphore.h"
#include "objects/obj_process.h"
#include "objects/object_table.h"
#include "objects/obj_thread.h"
#include "objects/obj_pipe.h"
#include "objects/obj_shared_mem.h"
#include "objects/obj_timer.h"
#include "time/time_manager.h"
#include "console/print_out.h"
#include "kernel_panic.h"

#define MAX_SYSCALL_ARGS 7

typedef struct syscall_handler_t
{
	proc_list_t * process_list;
	registry_t * reg;
	scheduler_t * scheduler;
	time_manager_t * time_manager;
	alarm_manager_t * alarm_manager;
	loader_t * loader;
} syscall_handler_t;

static inline object_number_t syscall_get_thread_oid(const thread_t * const thread)
{
	return thread_get_object_no(thread);
}

static inline object_thread_t * syscall_get_thread_object(
		const process_t * const process,
		const thread_t * const thread) __attribute__((always_inline));

static inline object_thread_t * syscall_get_thread_object(
		const process_t * const process,
		const thread_t * const thread)
{
	return (object_thread_t*)obj_get_object(
			process_get_object_table(process),
			syscall_get_thread_oid(thread));
}

static inline return_t syscall_delete_object(
		const process_t * const process,
		const object_number_t obj_no) __attribute__((always_inline));

static inline return_t syscall_delete_object(
		const process_t * const process,
		const object_number_t obj_no)
{
	return obj_remove_object(
			process_get_object_table(process),
			obj_no);
}

static inline object_sema_t * syscall_get_sema_obj(
		process_t * const process,
		object_number_t sema_id) __attribute__((always_inline));

static inline object_sema_t * syscall_get_sema_obj(
		process_t * const process,
		object_number_t sema_id)
{
	return obj_cast_semaphore(
			obj_get_object(
					process_get_object_table(process),
					sema_id));
}

syscall_handler_t * create_handler(
		mem_pool_info_t * const pool,
		proc_list_t * const proc_list,
		registry_t * const reg,
		scheduler_t * const scheduler,
		time_manager_t * const tm,
		alarm_manager_t * const am,
		loader_t * const loader)
{
	syscall_handler_t * const sys = mem_alloc(pool, sizeof(syscall_handler_t));
	if (sys)
	{
		sys->process_list = proc_list;
		sys->reg = reg;
		sys->scheduler = scheduler;
		sys->time_manager = tm;
		sys->alarm_manager = am;
		sys->loader = loader;
	}
	return sys;
}

static mem_t virtual_to_real(
		process_t * process,
		mem_t address)
{
	/* TODO the below code needs fixing */
	const bool_t is_kernel = process_is_kernel(process);
	mem_t result;
	if (is_kernel)
	{
		result = ((address >= VIRTUAL_ADDRESS_SPACE(is_kernel)) ? process_virt_to_real(process, address) : address);
	}
	else
	{
		result = process_virt_to_real(process, address);
	}
	return result;
}

static return_t syscall_tst(tgt_context_t * const context) {
	if (tgt_get_syscall_param(context, 1) != SYSCALL_TEST_1 || tgt_get_syscall_param(context, 2) != SYSCALL_TEST_2
			|| tgt_get_syscall_param(context, 3) != SYSCALL_TEST_3 || tgt_get_syscall_param(context, 4) != SYSCALL_TEST_4
			|| tgt_get_syscall_param(context, 5) != SYSCALL_TEST_5 || tgt_get_syscall_param(context, 6) != SYSCALL_TEST_6
			|| tgt_get_syscall_param(context, 7) != SYSCALL_TEST_7)
	{
		kernel_panic();
	}
	return NO_ERROR;
}

static return_t syscall_create_process(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		process_t * const parent) {
	process_t * process = NULL;
	return_t ret = proc_create_process(
			handler->process_list,
			(const char*) virtual_to_real(parent, tgt_get_syscall_param(context, 1)),
			MAIN_THREAD_NAME,
			(thread_entry_point*) (virtual_to_real(parent, tgt_get_syscall_param(context, 2))),
			(const uint8_t) tgt_get_syscall_param(context, 3),
			(tinker_meminfo_t* const ) virtual_to_real(parent, tgt_get_syscall_param(context, 4)),
			(uint32_t) tgt_get_syscall_param(context, 5),
			(process_t**) &process);
	*((object_number_t*) virtual_to_real(parent, tgt_get_syscall_param(context, 6))) = process_get_oid(process);
	return ret;
}

static return_t syscall_create_thread(
		tgt_context_t * const context,
		process_t * const process) {
	thread_t * thread = NULL;
	return_t ret = proc_create_thread(
			process,
			(const char*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
			(thread_entry_point*) (virtual_to_real(process, tgt_get_syscall_param(context, 2))),
			(const uint8_t) tgt_get_syscall_param(context, 3),
			(const uint32_t) tgt_get_syscall_param(context, 4),
			(uint32_t) tgt_get_syscall_param(context, 5),
			NULL,
			(thread_t**) &thread);
	*((object_number_t*) virtual_to_real(process, tgt_get_syscall_param(context, 6))) = thread_get_object_no(thread);
	return ret;
}

static return_t syscall_mmap(tgt_context_t* const context, process_t* const process) {
	mem_t virtual = 0;
	const mem_t real = virtual_to_real(process, tgt_get_syscall_param(context, 1));
	return_t ret;
	if (real == 0)
	{
		ret = MMAP_NOT_ALLOWED_AT_ZERO;
	}
	else
	{
		ret = process_allocate_vmem(process,
				real,
				(mem_t) (tgt_get_syscall_param(context, 2)),
				(mmu_memory_t) (tgt_get_syscall_param(context, 3)),
				(mmu_privilege_t) (tgt_get_syscall_param(context, 4)),
				MMU_READ_WRITE,
				&virtual,
				"MMAP");
	}
	*((mem_t*) virtual_to_real(process, tgt_get_syscall_param(context, 5))) = virtual;
	return ret;
}

static return_t syscall_sbrk(
		tgt_context_t* const context,
		thread_t* const this_thread) {
	process_t* const process = thread_get_parent(this_thread);
	void** base = (void**)virtual_to_real(process, tgt_get_syscall_param(context, 1));
	*base = mem_realloc(
			process_get_user_mem_pool(process),
			*base,
			tgt_get_syscall_param(context, 2));
	return_t ret;
	if (*base)
	{
		ret = NO_ERROR;
	}
	else
	{
		ret = OUT_OF_MEMORY;
	}
	return ret;
}

static return_t syscall_debug(
		tgt_context_t* const context,
		thread_t* const this_thread) {
	mem_t loc = virtual_to_real(thread_get_parent(this_thread), tgt_get_syscall_param(context, 1));
	const char * const msg = (const char*const )loc;
	if (is_debug_enabled(SYSCALL))
	{
		debug_print(SYSCALL, "Syscall: Debug of message at %x from %x [%s]\n", msg, tgt_get_syscall_param(context, 1), msg);
	}
	if (msg)
	{
		print_time();
		print_out_len(msg, tgt_get_syscall_param(context, 2));
	}
	return NO_ERROR;
}

static return_t syscall_thread_priority(
		tgt_context_t* const context,
		thread_t* const this_thread) {
	process_t* const process = thread_get_parent(this_thread);
	object_number_t thread_no = (object_number_t) tgt_get_syscall_param(context, 1);
	priority_t* priority = (priority_t*) virtual_to_real(process, tgt_get_syscall_param(context, 2));
	return_t ret;
	if (thread_no && priority) {
		object_table_t* const table = process_get_object_table(process);
		object_t* const obj = obj_get_object(table, thread_no);
		object_thread_t* thread_obj = NULL;
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
	return ret;
}

static return_t syscall_get_thread_id(
		tgt_context_t* const context,
		thread_t* const this_thread)
{
	const object_number_t objno = syscall_get_thread_oid(this_thread);
	object_number_t * oid = (object_number_t*) virtual_to_real(
			thread_get_parent(this_thread),
			tgt_get_syscall_param(context, 1));
	if (oid)
	{
		*oid = objno;
	}
	return NO_ERROR;
}

static return_t syscall_exit_thread(
		thread_t* const this_thread)
{
	return_t ret;
	process_t* const parent = thread_get_parent(this_thread);
	const object_table_t* const table = process_get_object_table(parent);
	object_thread_t* const thread_obj = obj_cast_thread(
			obj_get_object(table, syscall_get_thread_oid(this_thread)));
	if (thread_obj)
	{
		object_process_t* const proc_obj = obj_cast_process(
				obj_get_object(table, process_get_oid(parent)));
		if (proc_obj)
		{
			ret = obj_process_thread_exit(proc_obj, thread_obj);
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

static return_t syscall_wait_thread(
		syscall_handler_t* const handler,
		thread_t* const this_thread)
{
	thread_set_state(this_thread, THREAD_WAITING);
	sch_notify_pause_thread(handler->scheduler, this_thread);
	return NO_ERROR;
}

static return_t syscall_create_sema(
		syscall_handler_t* const handler,
		tgt_context_t* const context,
		thread_t* const this_thread)
{
	process_t * const proc = thread_get_parent(this_thread);
	object_number_t * const obj = (object_number_t*)virtual_to_real(thread_get_parent(this_thread), tgt_get_syscall_param(context, 1));
	return_t ret;
	if (obj)
	{
		ret = obj_create_semaphore(
				handler->reg, proc,
				obj,
				(char*) virtual_to_real(proc, tgt_get_syscall_param(context, 2)),
				(const uint32_t) tgt_get_syscall_param(context, 3));
	}
	else
	{
		ret = PARAMETERS_INVALID;
	}
	return ret;
}

static return_t syscall_open_sema(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t * const proc = thread_get_parent(this_thread);
	return obj_open_semaphore(
			handler->reg,
			proc,
			(object_number_t*) virtual_to_real(proc, tgt_get_syscall_param(context, 1)),
			(char*) virtual_to_real(proc, tgt_get_syscall_param(context, 2)));
}

static return_t syscall_get_sema(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t * const proc = thread_get_parent(this_thread);
	object_thread_t* const thread_obj = syscall_get_thread_object(proc, this_thread);
	object_sema_t * const sema_obj = syscall_get_sema_obj(
			proc,
			(object_number_t) tgt_get_syscall_param(context, 1));
	return_t ret;
	if (sema_obj && thread_obj)
	{
		ret = obj_get_semaphore(thread_obj, sema_obj);
	}
	else
	{
		ret = INVALID_OBJECT;
	}
	return ret;
}

static return_t syscall_release_sema(
		tgt_context_t * const context,
		thread_t const * this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	object_thread_t* const thread_obj = syscall_get_thread_object(process, this_thread);
	object_sema_t* const sema_obj = syscall_get_sema_obj(process, (object_number_t) tgt_get_syscall_param(context, 1));
	return_t ret;
	if (sema_obj && thread_obj)
	{
		ret = obj_release_semaphore(thread_obj, sema_obj);
	}
	else
	{
		ret = INVALID_OBJECT;
	}
	return ret;
}

static return_t syscall_close_sema(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t * const process = thread_get_parent(this_thread);
	object_number_t sema_id = (object_number_t) tgt_get_syscall_param(context, 1);
	object_sema_t * const sema_obj = syscall_get_sema_obj(process, sema_id);
	return_t ret;
	if (sema_obj)
	{
		ret = object_delete_semaphore(sema_obj);
		if (ret == NO_ERROR)
		{
			ret = syscall_delete_object(process, sema_id);
		}
	}
	else
	{
		ret = INVALID_OBJECT;
	}
	return ret;
}

static return_t syscall_create_pipe(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	return obj_create_pipe(
			handler->reg,
			process,
			(object_number_t*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
			(const char*) virtual_to_real(process, tgt_get_syscall_param(context, 2)),
			(const tinker_pipe_direction_t) tgt_get_syscall_param(context, 3),
			(const uint32_t) tgt_get_syscall_param(context, 4),
			(const uint32_t) tgt_get_syscall_param(context, 5));
}

static return_t syscall_delete_pipe(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	object_number_t pipe_id = tgt_get_syscall_param(context, 1);
	object_table_t* const table = process_get_object_table(process);
	object_pipe_t* const pipe = obj_cast_pipe(obj_get_object(table, pipe_id));
	return_t ret = obj_delete_pipe(pipe);
	if (ret == NO_ERROR)
	{
		ret = syscall_delete_object(process, pipe_id);
	}
	return ret;
}

static return_t syscall_open_pipe(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	return obj_open_pipe(
			handler->reg,
			process,
			syscall_get_thread_object(process, this_thread),
			(object_number_t*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
			(const char*) virtual_to_real(process, tgt_get_syscall_param(context, 2)),
			(const tinker_pipe_direction_t) tgt_get_syscall_param(context, 3),
			(const uint32_t) tgt_get_syscall_param(context, 4),
			(const uint32_t) tgt_get_syscall_param(context, 5));
}

static return_t syscall_close_pipe(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t * const process = thread_get_parent(this_thread);
	object_number_t pipe_id = tgt_get_syscall_param(context, 1);
	object_pipe_t* const pipe = obj_cast_pipe(
			obj_get_object(
					process_get_object_table(
							process),
					pipe_id));
	// FIXME This will release memory but isn't right,
	// close needs to remove the item from the senders list etc
	return_t ret = obj_delete_pipe(pipe);
	if (ret == NO_ERROR)
	{
		ret = syscall_delete_object(process, pipe_id);
	}
	return ret;
}

static return_t syscall_send_message(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	object_pipe_t* const pipe = obj_cast_pipe(
			obj_get_object(process_get_object_table(process),
					(object_number_t) tgt_get_syscall_param(context, 1)));
	return obj_pipe_send_message(pipe,
			syscall_get_thread_object(process, this_thread),
			(tinker_pipe_send_kind_t) tgt_get_syscall_param(context, 2),
			(void*) virtual_to_real(process, tgt_get_syscall_param(context, 3)),
			(const uint32_t) tgt_get_syscall_param(context, 4),
			(const bool_t) tgt_get_syscall_param(context, 5));
}

static return_t syscall_receive_message(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	object_table_t* const table = process_get_object_table(process);
	object_pipe_t* const pipe = obj_cast_pipe(
			obj_get_object(table, (object_number_t) tgt_get_syscall_param(context, 1)));
	void* msg = (void*) virtual_to_real(process, tgt_get_syscall_param(context, 2));
	uint32_t* msg_size = (uint32_t*) virtual_to_real(process, tgt_get_syscall_param(context, 3));
	uint32_t max_size = tgt_get_syscall_param(context, 4);

	const return_t ret = obj_pipe_receive_message(
			pipe,
			syscall_get_thread_object(process, this_thread),
			(void**) msg,
			msg_size,
			max_size,
			(const bool_t) tgt_get_syscall_param(context, 5));

	return ret;
}

static return_t syscall_create_shm(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	return obj_create_shm(handler->reg, process,
			(object_number_t*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
			(char*) virtual_to_real(process, tgt_get_syscall_param(context, 2)),
			(uint32_t) tgt_get_syscall_param(context, 3),
			(void**) virtual_to_real(process, tgt_get_syscall_param(context, 4)));
}

static return_t syscall_open_shm(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	return obj_open_shm(handler->reg, thread_get_parent(this_thread),
			(object_number_t*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
			(char*) virtual_to_real(process, tgt_get_syscall_param(context, 2)),
			(uint32_t) tgt_get_syscall_param(context, 3),
			(void**) virtual_to_real(process, tgt_get_syscall_param(context, 4)));
}

static return_t syscall_create_timer(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	return obj_create_timer(handler->scheduler, handler->alarm_manager,
			thread_get_parent(this_thread),
			(object_number_t*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
			(const priority_t) tgt_get_syscall_param(context, 2),
			(const uint32_t) tgt_get_syscall_param(context, 3),
			(const uint32_t) tgt_get_syscall_param(context, 4),
			(tinker_timer_callback_t*) virtual_to_real(process,
					tgt_get_syscall_param(context, 5)),
			(void*) virtual_to_real(process, tgt_get_syscall_param(context, 6)));
}

static return_t syscall_destroy_shm(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	object_number_t oid = tgt_get_syscall_param(context, 1);
	process_t* const process = thread_get_parent(this_thread);
	object_table_t* const table = process_get_object_table(process);
	object_shm_t* const shm = obj_cast_shm(
			(object_t*) obj_get_object(table, oid));
	return_t ret = obj_delete_shm(shm);
	if (ret == NO_ERROR)
	{
		ret = syscall_delete_object(process, oid);
	}
	return ret;
}

static return_t syscall_cancel_timer(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	object_table_t* const table = process_get_object_table(
			thread_get_parent(this_thread));
	object_timer_t* const timer = obj_cast_timer(
			(object_t*) obj_get_object(table,
					(object_number_t) tgt_get_syscall_param(context, 1)));
	return obj_cancel_timer(timer);
}

static return_t syscall_delete_timer(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	object_number_t oid = tgt_get_syscall_param(context, 1);
	process_t* const process = thread_get_parent(this_thread);
	object_table_t* const table = process_get_object_table(process);
	object_timer_t* const timer = obj_cast_timer(
			(object_t*) obj_get_object(table, oid));
	return_t ret = obj_delete_timer(timer);
	if (ret == NO_ERROR)
	{
		ret = syscall_delete_object(process, oid);
	}
	return ret;
}

static return_t syscall_get_time(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	tinker_time_t* time = (tinker_time_t*) virtual_to_real(process,
			tgt_get_syscall_param(context, 1));
	return_t ret;
	if (time)
	{
		time_get_system_time(handler->time_manager, time);
		ret = NO_ERROR;
	}
	else
	{
		ret = PARAMETERS_NULL;
	}
	return ret;
}

static return_t syscall_sleep(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	const tinker_time_t* const duration = (const tinker_time_t*) virtual_to_real(
			process, tgt_get_syscall_param(context, 1));
	object_thread_t* const thread_obj = syscall_get_thread_object(
			thread_get_parent(this_thread), this_thread);
	return obj_thread_sleep(thread_obj, duration);
}

static return_t syscall_wfi(void)
{
	tgt_wait_for_interrupt();
	return NO_ERROR;
}

static return_t syscall_load_thread(
		tgt_context_t * const context,
		thread_t const * this_thread)
{
	/* uses the current thread from the scheduler */
	tgt_prepare_context(context, this_thread, NULL);
	return NO_ERROR;
}

static return_t syscall_load_elf(
		syscall_handler_t* const handler,
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	return load_elf(
		handler->loader,
		(void*) virtual_to_real(process, tgt_get_syscall_param(context, 1)),
		(const char*) virtual_to_real(process, tgt_get_syscall_param(context, 2)),
		tgt_get_syscall_param(context, 3),
		tgt_get_syscall_param(context, 4));
}

return_t syscall_get_pid(
		tgt_context_t * const context,
		thread_t* const this_thread)
{
	process_t* const process = thread_get_parent(this_thread);
	uint32_t* pid = (uint32_t*) virtual_to_real(process, tgt_get_syscall_param(context, 1));
	*pid = process_get_pid(process);
	return NO_ERROR;
}

void syscall_handle_system_call(
		syscall_handler_t * const handler,
		tgt_context_t * const context)
{
	const syscall_function_t api = (syscall_function_t)tgt_get_syscall_param(context, 0);
	return_t ret = UNKNOWN_ERROR;
	thread_t * const this_thread = sch_get_current_thread(handler->scheduler);

	if (is_debug_enabled(SYSCALL))
	{
		debug_print(SYSCALL, "Syscall: API %d from %s sp=0x%8x|usp=0x%8x|upc=0x%8x\n", api, thread_get_name(this_thread),
				tgt_get_stack_pointer(),
				tgt_get_context_stack_pointer(context),
				tgt_get_pc(context));
	}

	/*
	 * This could use a jump table but I think in this
	 * case this is a bit more readable.
	 */
	switch (api)
	{
		case SYSCALL_TEST:
			ret = syscall_tst(context);
			break;
		case SYSCALL_CREATE_PROCESS:
			ret = syscall_create_process(handler, context, thread_get_parent(this_thread));
			break;
		case SYSCALL_CREATE_THREAD:
			ret = syscall_create_thread(context, thread_get_parent(this_thread));
			break;
		case SYSCALL_MMAP:
			ret = syscall_mmap(context, thread_get_parent(this_thread));
			break;
		case SYSCALL_SBRK:
			ret = syscall_sbrk(context, this_thread);
			break;
		case SYSCALL_DEBUG:
			ret = syscall_debug(context, this_thread);
			break;
		case SYSCALL_THREAD_PRIORITY:
			ret = syscall_thread_priority(context, this_thread);
			break;
		case SYSCALL_THREAD_OBJECT:
			ret = syscall_get_thread_id(context, this_thread);
			break;
		case SYSCALL_EXIT_THREAD:
			ret = syscall_exit_thread(this_thread);
			break;
		case SYSCALL_WAIT_THREAD:
			ret = syscall_wait_thread(handler, this_thread);
			break;
		case SYSCALL_CREATE_SEMAPHORE:
			ret = syscall_create_sema(handler, context, this_thread);
			break;
		case SYSCALL_OPEN_SEMAPHORE:
			ret = syscall_open_sema(handler, context, this_thread);
			break;
		case SYSCALL_GET_SEMAPHORE:
			ret = syscall_get_sema(context, this_thread);
			break;
		case SYSCALL_RELEASE_SEMAPHORE:
			ret = syscall_release_sema(context, this_thread);
			break;
		case SYSCALL_CLOSE_SEMAPHORE:
			ret = syscall_close_sema(context, this_thread);
			break;
		case SYSCALL_CREATE_PIPE:
			ret = syscall_create_pipe(handler, context, this_thread);
			break;
		case SYSCALL_DELETE_PIPE:
			ret = syscall_delete_pipe(context, this_thread);
			break;
		case SYSCALL_OPEN_PIPE:
			ret = syscall_open_pipe(handler, context, this_thread);
			break;
		case SYSCALL_CLOSE_PIPE:
			ret = syscall_close_pipe(context, this_thread);
			break;
		case SYSCALL_SEND_MESSAGE:
			ret = syscall_send_message(context, this_thread);
			break;
		case SYSCALL_RECEIVE_MESSAGE:
			ret = syscall_receive_message(context, this_thread);
			break;
		case SYSCALL_CREATE_SHM:
			ret = syscall_create_shm(handler, context, this_thread);
			break;
		case SYSCALL_OPEN_SHM:
			ret = syscall_open_shm(handler, context, this_thread);
			break;
		case SYSCALL_DESTROY_SHM:
			ret = syscall_destroy_shm(context, this_thread);
			break;
		case SYSCALL_CREATE_TIMER:
			ret = syscall_create_timer(handler, context, this_thread);
			break;
		case SYSCALL_CANCEL_TIMER:
			ret = syscall_cancel_timer(context, this_thread);
			break;
		case SYSCALL_DELETE_TIMER:
			ret = syscall_delete_timer(context, this_thread);
			break;
		case SYSCALL_GET_TIME:
			ret = syscall_get_time(handler, context, this_thread);
			break;
		case SYSCALL_SLEEP:
			ret = syscall_sleep(context, this_thread);
			break;
		case SYSCALL_WFI:
			ret = syscall_wfi();
			break;
		case SYSCALL_LOAD_THREAD:
			/* uses the current thread from the scheduler */
			ret = syscall_load_thread(context, this_thread);
			break;
		case SYSCALL_LOAD_ELF:
			ret = syscall_load_elf(handler, context, this_thread);
			break;
		case SYSCALL_GET_PID:
			ret = syscall_get_pid(context, this_thread);
			break;
		case SYSCALL_UNKNOWN:
		case MAX_SYSCALL:
		case SYSCALL_RECEIVED_MESSAGE_NOT_USED:
		default:
			ret = ERROR_UNKNOWN_SYSCALL;
			break;
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
	/**
	 * Check to see if the current thread that made the syscall is
	 * still running - if it's not running anymore then we need to
	 * run the scheduler to perform a context switch on our way out
	 */
	bool_t save_state = true;
	if ( (state != THREAD_SYSTEM) &&
		 (state != THREAD_RUNNING) )
	{
		/* save the existing data - i.e. the return & run the scheduler */
		debug_print(SYSCALL, "Syscall: Saving ret code %d into context %x\n", ret, context);
		tgt_set_syscall_return(context, ret);
		sch_set_context_for_next_thread(handler->scheduler, context, state);
		save_state = false;
	}

	/* This will over-ride the result of a system-call
	 * if the exception occurs just after the system call has been made. */
	if (api == SYSCALL_LOAD_THREAD
			|| api == SYSCALL_EXIT_THREAD)
	{
		debug_prints(SYSCALL, "Syscall: Initial Kernel mode call to start scheduler\n");
		bsp_enable_schedule_timer();
	}
	else if (save_state)
	{
		debug_print(SYSCALL, "Syscall: Saving ret code %d into context %x\n", ret, context);
		tgt_set_syscall_return(context, ret);
	}
	else
	{
		debug_print(SYSCALL, "Syscall: Not saving context %x\n", context);
	}
	if (is_debug_enabled(SYSCALL))
	{
		debug_print(SYSCALL, "Syscall: %c, API %d RET %d sp=0x%8x|usp=0x%8x|upc=0x%8x\n", api, ret,
				thread_get_name(sch_get_current_thread(handler->scheduler)),
				tgt_get_stack_pointer(),
				tgt_get_context_stack_pointer(context),
				tgt_get_pc(context));
	}
}
