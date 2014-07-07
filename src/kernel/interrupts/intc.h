/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef INTC_H_
#define INTC_H_

#include "kernel/memory/mem_pool.h"
#include "kernel/devices/kernel_device.h"
#include "kernel/devices/kernel_intc_device.h"
#include "kernel/objects/obj_pipe.h"

typedef struct intc_t intc_t;

typedef enum
{
	/* nothing hanging off the pin */
	EMPTY_DEVICE = 0,
	/* a child intc is hanging off the pin */
	CHILD_INTC,
	/* a device isr is hanging off the pin */
	DEVICE_INTC,
	/* a pipe is hanging off the pin */
	PIPE_INTC
} intc_device_type;

intc_t * intc_create(mem_pool_info_t * const pool, kernel_intc_t * const kernel_intc);

void intc_delete(intc_t * const intc);

void intc_add_child(intc_t * const intc, const uint32_t cause, const intc_t * child);

void intc_add_device(intc_t * const intc, const uint32_t cause, const kernel_device_t * device);

void intc_add_pipe(intc_t * const intc, const uint32_t cause, const object_pipe_t * pipe);

error_t intc_handle(const intc_t * const intc);

error_t intc_setup(
		intc_t * const intc,
		const uint32_t cause,
		const intc_priority_t priority,
		const intc_detection_type detection,
		const intc_active_type edge_type);

error_t intc_enable(intc_t * const intc, const uint32_t cause);

error_t intc_mask(intc_t * const intc, const uint32_t cause);

#endif /* INTC_H_ */
