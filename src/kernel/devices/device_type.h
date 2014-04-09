/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef DEVICE_TYPE_H_
#define DEVICE_TYPE_H_

#include "arch/tgt_types.h"
#include "sos_api_errors.h"

struct __kernel_device;

typedef struct __kernel_device_info
{
	uint8_t		major_version;
	uint8_t		minor_version;
	uint8_t		revision;
	uint32_t	build;
}__kernel_device_info_t;

typedef error_t (__kernel_device_initialise)(
		struct __kernel_device * device_info,
		void * param,
		const uint32_t param_size);

typedef error_t (__kernel_device_control)(
		void * usr_data, uint32_t code);

typedef error_t (__kernel_device_write_register)(
		const void * const usr_data,
		const uint32_t id,
		const uint32_t val);

typedef error_t (__kernel_device_read_register)(
		const void * const usr_data, const uint32_t id, uint32_t * const val);

typedef error_t (__kernel_device_write_buffer)(
		const void * const usr_data,
		const uint32_t dst,
		const void * const src,
		const uint32_t src_size);

typedef error_t (__kernel_device_read_buffer)(
		const void * const usr_data,
		const uint32_t src,
		void * const dst,
		const uint32_t dst_size);

typedef error_t (__kernel_device_isr)(
		const void * const usr_data,
		const uint32_t vector);

typedef struct __kernel_device
{
	__kernel_device_info_t		 	info;
	__kernel_device_initialise * 	initialise;
	__kernel_device_control	*		control;
	__kernel_device_write_register *write_register;
	__kernel_device_read_register *	read_register;
	__kernel_device_write_buffer *	write_buffer;
	__kernel_device_read_buffer *	read_buffer;
	__kernel_device_isr * 			isr;
	const void * user_data;
} __kernel_device_t;

typedef bool_t __kernel_intc_get_cause(uint32_t * const cause, const void * const user_data);
typedef void   __kernel_intc_ack_cause(uint32_t const cause, const void * const user_data);
typedef void   __kernel_intc_enable_cause(const uint32_t cause, const void * const user_data);
typedef void   __kernel_intc_mask_cause(const uint32_t cause, const void * const user_data);

typedef struct __kernel_intc
{
	__kernel_intc_get_cause *		get_cause;
	__kernel_intc_ack_cause * 		ack_cause;
	__kernel_intc_enable_cause *	enable_cause;
	__kernel_intc_mask_cause * 		mask_cause;
	const void * user_data;
} __kernel_intc_t;

#endif /* DEVICE_TYPE_H_ */
