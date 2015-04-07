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

error_t kernel_device_map_memory
	(const uint32_t addr,
	 const uint32_t size,
	 const mmu_memory_t type,
	 uint32_t * const virt)
{
	return process_allocate_vmem(
			kernel_get_process(),
			addr,
			size,
			type,
			MMU_KERNEL_ACCESS,
			MMU_READ_WRITE,
			virt);
	// TODO if successful iterate around the processes
	// and add it to all non-kernel spaces too
}
