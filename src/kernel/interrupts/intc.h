/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef INTC_H_
#define INTC_H_

#include "kernel/memory/mem_pool.h"
#include "kernel/devices/device_type.h"
#include "kernel/objects/obj_pipe.h"

typedef struct __intc_t __intc_t;

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
} __intc_device_type;

__intc_t * __intc_create(__mem_pool_info_t * const pool, __kernel_intc_t * const kernel_intc);

void __intc_delete(__intc_t * const intc);

void __intc_add_child(__intc_t * const intc, const uint32_t cause, const __intc_t * child);

void __intc_add_device(__intc_t * const intc, const uint32_t cause, const __kernel_device_t * device);

void __intc_add_pipe(__intc_t * const intc, const uint32_t cause, const __object_pipe_t * pipe);

error_t __intc_handle(const __intc_t * const intc);

error_t __intc_enable(__intc_t * const intc, const uint32_t cause);

error_t __intc_mask(__intc_t * const intc, const uint32_t cause);

#endif /* INTC_H_ */
