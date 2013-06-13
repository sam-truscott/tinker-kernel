/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "opic_intc.h"

#include "arch/tgt.h"
#include "tgt_io.h"
#include "kernel/utils/util_memcpy.h"

/**
 * Write a value to a register on the device
 * @param usr_data The base address of the device
 * @param id The register id
 * @param val The value to write
 * @return Error
 */
static error_t opic_intc_write_register(const void * const usr_data, const uint32_t id, const uint32_t val);

/**
 * Read a value from a register on the device
 * @param usr_data The base address of the device
 * @param id  The register id
 * @param val The address of value to write
 * @return Error
 */
static error_t opic_intc_read_register(const void * const usr_data, const uint32_t id, uint32_t * const val);

/**
 * Device details
 */
static const __kernel_device_t OPIC_DEVICE_INFO =
{
		.info = {
				.major_version = 1,
				.minor_version = 0,
				.revision = 0,
				.build = 0
		},
		.write_register = opic_intc_write_register,
		.read_register = opic_intc_read_register,
};

/*
 * The procedures below are for the device interface
 */

void opic_intc_get_device(
		const opic_user_data_t * const user_data,
		__kernel_device_t * const device)
{
	__kernel_device_t new_device;
	__util_memcpy(
			&new_device,
			&OPIC_DEVICE_INFO,
			sizeof(OPIC_DEVICE_INFO));

	new_device.user_data = (void*)user_data;

	if ( device )
	{
		__util_memcpy(
				device,
				&new_device,
				sizeof(__kernel_device_t));
	}
}

error_t opic_intc_write_register(const void * const usr_data, const uint32_t id, const uint32_t val)
{
#ifdef OPIC_BIG_ENDIAN
		uint32_t new_val = val;
		new_val = SWAP_32(new_val);
		*(uint32_t*)((uint32_t)(usr_data) + id) = new_val;
#else
		*(uint32_t*)((uint32_t)(usr_data) + id) = val;
#endif

	return NO_ERROR;
}

error_t opic_intc_read_register(const void * const usr_data, const uint32_t id, uint32_t * const val)
{
	/* TODO driver_id should be checked */
#ifdef OPIC_BIG_ENDIAN
		uint32_t v;
		v = *(uint32_t*)((uint32_t)(usr_data) + id);
		v = SWAP_32(v);
		*val = v;
#else
		*val = *(uint32_t*)((uint32_t)(usr_data) + id);
#endif

	return NO_ERROR;
}

/*
 * The procedures below are for high level functions
 */
uint32_t opic_ack(
		const __kernel_device_t * const device)
{
	return __in_u32((const uint32_t*)(((const opic_user_data_t*)device->user_data)->base_address
			+(OPIC_PROC_BASE + INTERRUPT_ACK_REGISTER_N)));
}

void opic_end_isr(
		const __kernel_device_t * const device,
		const uint32_t port)
{
	__out_u32(((uint32_t*)((opic_user_data_t*)device->user_data)->base_address
			+ (OPIC_PROC_BASE + END_OF_INTERRUPT_REGISTER_N)),
			port);
}
