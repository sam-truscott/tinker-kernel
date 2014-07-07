/*
 * kernel_intc_device.h
 *
 *  Created on: 7 Jul 2014
 *      Author: sam
 */

#ifndef KERNEL_INTC_DEVICE_H_
#define KERNEL_INTC_DEVICE_H_

#include "arch/tgt_types.h"
#include "tinker_api_errors.h"

typedef enum
{
	INTC_UNKNOWN_DETECT = 0,
	INTC_EDGE,
	INTC_LEVEL
} intc_detection_type;

typedef enum
{
	INTC_UNKNOWN_ACTIVE = 0,
	INTC_ACTIVE_HIGH,
	INTC_ACTIVE_LOW
} intc_active_type;

typedef uint32_t intc_priority_t;

typedef void kernel_intc_setup_cause(
		const uint32_t cause,
		const intc_priority_t priority,
		const intc_detection_type detection,
		const intc_active_type edge_type,
		const void * const user_data);
typedef bool_t kernel_intc_get_cause(uint32_t * const cause, const void * const user_data);
typedef void   kernel_intc_ack_cause(const uint32_t cause, const void * const user_data);
typedef void   kernel_intc_enable_cause(const uint32_t cause, const void * const user_data);
typedef void   kernel_intc_mask_cause(const uint32_t cause, const void * const user_data);

typedef struct kernel_intc
{
	kernel_intc_setup_cause *	setup_cause;
	kernel_intc_get_cause *		get_cause;
	kernel_intc_ack_cause * 	ack_cause;
	kernel_intc_enable_cause *	enable_cause;
	kernel_intc_mask_cause * 	mask_cause;
	const void * user_data;
} kernel_intc_t;

#endif /* KERNEL_INTC_DEVICE_H_ */
