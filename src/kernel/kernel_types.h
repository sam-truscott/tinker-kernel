/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KERNEL_TYPES_H_
#define KERNEL_TYPES_H_

#include "config.h"
#include "arch/target_types.h"
#include "api/sos_api_types.h"
#include "api/sos_api_errors.h"

/*
 * ========================================
 * == Clock Types                        ==
 * ========================================
 */

#define ONE_SECOND_AS_NANOSECONDS	1000000000
#define ONE_MS_AS_NANOSECONDS 		1000000
#define ONE_US_AS_NANOSECONDS 		1000

typedef struct
{
	int32_t seconds;
	int32_t nanoseconds;
} __time_t;

#define __ZERO_TIME ((__time_t){0,0})

typedef uint64_t (__clock_get_nanoseconds_time)(void);

typedef struct
{
	__clock_get_nanoseconds_time * 		get_time;
} __clock_device_t;

typedef void (__timer_callback)(void);

typedef void (__timer_setup)(
		const void * const usr_data,
		const __time_t timeout,
		__timer_callback * const call_back);

typedef void (__timer_cancel)(
		const void * const usr_data);

typedef struct
{
	__timer_setup *		timer_setup;
	__timer_cancel *	timer_cancel;
	void * 				usr_data;
	uint32_t 			usr_data_size;
} __timer_t;

typedef void(__alarm_call_back)(
		const void * const usr_data,
		const uint32_t usr_data_param);

/*
 * ========================================
 * == Device Driver Types                ==
 * ========================================
 */

struct __kernel_device;

typedef struct __kernel_device_info
{
	uint8_t	major_version;
	uint8_t	minor_version;
	uint8_t	revision;
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

#endif /* KERNEL_TYPES_H_ */
