/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */


#include "kernel/devices/kernel_device.h"
#include "kernel/process/process.h"
#include "kernel/kernel_initialise.h"

static proc_list_t * proc_list = NULL;

void kernel_device_set_process_list(proc_list_t * const list)
{
	proc_list = list;
}

error_t kernel_device_map_memory
	(const uint32_t addr,
	 const uint32_t size,
	 const mmu_memory_t type,
	 uint32_t * const virt)
{
	process_t * const kernel_proc = proc_get_kernel_process(proc_list);
	const error_t result = process_allocate_vmem(
			kernel_proc,
			addr,
			size,
			type,
			MMU_KERNEL_ACCESS,
			MMU_READ_WRITE,
			virt);
	if (NO_ERROR == result)
	{
		process_list_it_t * const procs = proc_list_procs(proc_list);
		process_t * proc = NULL;
		if (procs && process_list_it_t_get(procs, &proc))
		{
			while (proc)
			{
				if (proc != kernel_proc)
				{
					process_allocate_vmem(
							proc,
							addr,
							size,
							type,
							MMU_KERNEL_ACCESS,
							MMU_READ_WRITE,
							virt);
				}
				process_list_it_t_next(procs, &proc);
			}
			process_list_it_t_delete(procs);
		}
	}
	return result;
}

void * kernel_device_malloc(
		const uint32_t size)
{
	return mem_alloc(mem_get_default_pool(), size);
}

object_pipe_t * kernel_isr_get_pipe(
		char * name)
{
	object_pipe_t * pipe = NULL;
	process_t * other_proc = NULL;
	object_pipe_t * other_pipe = NULL;
	object_number_t other_pipe_no = INVALID_OBJECT_ID;
	if (NO_ERROR == registry_get(kernel_get_registry(), name, &other_proc, &other_pipe_no))
	{
		const object_table_t * const table = process_get_object_table(other_proc);
		object_t * const other_obj = obj_get_object(table, other_pipe_no);
		if (other_obj)
		{
			other_pipe = obj_cast_pipe(other_obj);
			if (other_pipe)
			{
				pipe = other_pipe;
			}
		}
	}
	return pipe;
}

error_t kernel_isr_write_pipe(
		object_pipe_t * const pipe,
		void * buffer,
		uint32_t size)
{
	return obj_pipe_send_message(
			pipe,
			NULL,
			PIPE_TX_SEND_AVAILABLE,
			buffer,
			size,
			false);
}
