/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_DEVICE_H_
#define KERNEL_DEVICE_H_

#include "tgt_types.h"
#include "tinker_api_errors.h"
#include "memory/mem_section.h"
#include "process/process_list.h"
#include "objects/obj_pipe.h"

struct kernel_device;

typedef return_t (kernel_device_initialise)(
		struct kernel_device * device_info,
		void * param,
		const uint32_t param_size);

typedef return_t (kernel_device_control)(
		void * usr_data, uint32_t code);

typedef return_t (kernel_device_write_register)(
		const void * const usr_data,
		const uint32_t id,
		const uint32_t val);

typedef return_t (kernel_device_read_register)(
		const void * const usr_data, const uint32_t id, uint32_t * const val);

typedef return_t (kernel_device_write_buffer)(
		const void * const usr_data,
		const uint32_t dst,
		const void * const src,
		const uint32_t src_size);

typedef return_t (kernel_device_read_buffer)(
		const void * const usr_data,
		const uint32_t src,
		void * const dst,
		const uint32_t dst_size);

typedef return_t (kernel_device_isr)(
		const void * const usr_data,
		const uint32_t vector);

typedef struct kernel_device
{
	kernel_device_initialise * 		initialise;
	kernel_device_control	*		control;
	kernel_device_write_register *	write_register;
	kernel_device_read_register *	read_register;
	kernel_device_write_buffer *	write_buffer;
	kernel_device_read_buffer *		read_buffer;
	kernel_device_isr * 			isr;
	void * 							user_data;
} kernel_device_t;

// TODO remove this and use an object
void kernel_device_init(
		process_t * const kproc,
		registry_t * const reg,
		proc_list_t * const list);

return_t kernel_device_map_memory
	(const mem_t addr,
	 const mem_t size,
	 const mmu_memory_t type,
	 mem_t * const virt);

void * kernel_device_malloc(
		const mem_t size);

object_pipe_t * kernel_isr_get_pipe(
		const char * const name);

return_t kernel_isr_write_pipe(
		object_pipe_t * const pipe,
		void * buffer,
		uint32_t size);

#endif /* KERNEL_DEVICE_H_ */
