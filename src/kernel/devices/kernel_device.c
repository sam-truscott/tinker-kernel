/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */


#include "kernel/devices/kernel_device.h"
#include "kernel/kernel_initialise.h"
#include "kernel/process/process.h"

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
	process_t * const kernel_proc = kernel_get_process();
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
