/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "opic_intc.h"

#include "opic_private.h"
#include "arch/tgt.h"
#include "tgt_io.h"
#include "kernel/utils/util_memcpy.h"

//static error_t opic_intc_write_register(const void * const usr_data, const uint32_t id, const uint32_t val);

//static error_t opic_intc_read_register(const void * const usr_data, const uint32_t id, uint32_t * const val);

static bool_t opic_get(
		uint32_t * const cause,
		const void * const user_data)
{
	*cause = __in_u32(((uint32_t*)user_data) + (OPIC_PROC_BASE + INTERRUPT_ACK_REGISTER_N));
	return (*cause) > 0;
}

static void opic_ack(
		const uint32_t cause,
		const void * const user_data)
{
	__out_u32(((uint32_t*)user_data) + (OPIC_PROC_BASE + END_OF_INTERRUPT_REGISTER_N), cause);
}

static void opic_mask(
		const uint32_t cause,
		const void * const user_data)
{
	// TODO: ISU register setup
	if (cause && user_data){}
}

static void opic_enable(
		const uint32_t cause,
		const void * const user_data)
{
	// TODO: ISU register setup
	if (cause && user_data){}
}

__intc_t* opic_intc_create(
		__mem_pool_info_t * const pool,
		const uint8_t * const base_address)
{
	__intc_t * intc = NULL;
	__kernel_intc_t * const intc_device = (__kernel_intc_t*)__mem_alloc(pool, sizeof(__kernel_intc_t));
	if (intc_device)
	{
		intc_device->get_cause = opic_get;
		intc_device->ack_cause = opic_ack;
		intc_device->enable_cause = opic_enable;
		intc_device->mask_cause = opic_mask;
		intc_device->user_data = base_address;
		intc = __intc_create(pool, intc_device);
	}
	else
	{
		intc = NULL;
	}
	return intc;
}

/*
error_t opic_intc_write_register(const void * const usr_data, const uint32_t id, const uint32_t val)
{
#if defined(OPIC_BIG_ENDIAN)
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
#if defined(OPIC_BIG_ENDIAN)
		uint32_t v;
		v = *(uint32_t*)((uint32_t)(usr_data) + id);
		v = SWAP_32(v);
		*val = v;
#else
		*val = *(uint32_t*)((uint32_t)(usr_data) + id);
#endif

	return NO_ERROR;
}
*/
