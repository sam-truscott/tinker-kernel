/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#include "opic_intc.h"

#include "kernel/device/device_manager.h"

/**
 * Write a value to a register on the device
 * @param usr_data The base address of the device
 * @param id The register id
 * @param val The value to write
 * @return Error
 */
static error_t opic_intc_write_register(void * usr_data, uint32_t id, uint32_t val);

/**
 * Read a value from a register on the device
 * @param usr_data The base address of the device
 * @param id  The register id
 * @param val The address of value to write
 * @return Error
 */
static error_t opic_intc_read_register(void * usr_data, uint32_t id, uint32_t * val);

/**
 * Endian swap routine
 * @param var The variable to swap
 */
static void opic_swap_endianness(uint32_t* var);

/**
 * Device details
 */
static const __kernel_device_t UART16550_DEVICE_INFO =
{
		{1,0,0,0, {"opic_intc\0"}},
		NULL,
		NULL,
		opic_intc_write_register,
		opic_intc_read_register,
		NULL,
		NULL,
		NULL,
		NULL
};

/*
 * The procedures below are for the device interface
 */

void opic_intc_get_device(void * base_address, __kernel_device_t * device)
{
	__kernel_device_t new_device = UART16550_DEVICE_INFO;

	new_device.user_data = base_address;

	if ( device )
	{
		*device = new_device;
	}
}

error_t opic_intc_write_register(void * usr_data, uint32_t id, uint32_t val)
{
#ifdef OPIC_BIG_ENDIAN
		uint32_t new_val = val;
		opic_swap_endianness(&new_val);
		*(uint32_t*)((uint32_t)(usr_data) + id) = new_val;
#else
		*(uint32_t*)((uint32_t)(usr_data) + id) = val;
#endif

	return NO_ERROR;
}

error_t opic_intc_read_register(void * usr_data, uint32_t id, uint32_t * val)
{
	/* TODO driver_id should be checked */
#ifdef OPIC_BIG_ENDIAN
		uint32_t v;
		v = *(uint32_t*)((uint32_t)(usr_data) + id);
		opic_swap_endianness(&v);
		*val = v;
#else
		*val = *(uint32_t*)((uint32_t)(usr_data) + id);
#endif

	return NO_ERROR;
}

static void opic_swap_endianness(uint32_t* var)
{
	uint32_t new_val = 0;
	uint8_t* overlay = (uint8_t*)var;

	new_val += (overlay[3] << 24);
	new_val += (overlay[2] << 16);
	new_val += (overlay[1] << 8);
	new_val += (overlay[0]);

	*var = new_val;
}

/*
 * The procedures below are for high level functions
 */
uint32_t opic_ack(uint32_t device_id)
{
	uint32_t val = 0;
	__devm_read_register(
			device_id,
			OPIC_PROC_BASE + INTERRUPT_ACK_REGISTER_N,
			&val);
	return val;
}

void opic_end_isr(uint32_t device_id, uint32_t port)
{
	__devm_write_register(
			device_id,
			OPIC_PROC_BASE + END_OF_INTERRUPT_REGISTER_N,
			port);
}
