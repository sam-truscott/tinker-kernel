/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "opic_intc.h"

#include "opic_private.h"
#include "tgt.h"
#include "tgt_io.h"
#include "utils/util_memcpy.h"

static void opic_swap_endianness(uint32_t* var)
{
	const uint32_t copy = *var;
	*var = ((copy & 0xFF) << 24) |
			((copy & 0xFF00) << 8) |
			((copy & 0xFF0000) >> 8) |
			((copy & 0xFF000000) >> 24);
}

static bool_t opic_get(
		uint32_t * const cause,
		const void * const user_data)
{
	uint32_t * addr = (uint32_t*)
			(((char*)user_data) +
					(OPIC_PROC_BASE
							+ (INTERRUPT_ACK_REGISTER_N)));
	*cause = in_u32(addr);
#if defined (OPIC_BIG_ENDIAN)
		opic_swap_endianness(cause);
#endif  // OPIC_BIG_ENDIAN
	return (*cause) > 0;
}

static void opic_ack(
		const uint32_t cause,
		const void * const user_data)
{
	uint32_t cause_copy = cause;
#if defined (OPIC_BIG_ENDIAN)
		opic_swap_endianness(&cause_copy);
#endif  // OPIC_BIG_ENDIAN
	uint32_t * addr = (uint32_t*)
			(((char*)user_data) +
					(OPIC_PROC_BASE
							+ (END_OF_INTERRUPT_REGISTER_N)));
	out_u32(addr, 0/*cause_copy*/);
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
	if (cause && user_data)
	{
		out_u32((uint32_t*)
				(((char*)user_data) +
						(OPIC_ISU_BASE
								+ (INTC_SRC_N_DEST_REGISTER
										+ (cause * ISU_BLOCK_SIZE)))),
										0xFFFFFFFF); //FIXME TODO Find out why 1 doesn't work
	}
}

static void opic_setup(
		const uint32_t cause,
		const intc_priority_t priority,
		const intc_detection_type detection,
		const intc_active_type edge_type,
		const void * const user_data)
{
	if (cause && user_data)
	{
		uint32_t value = (cause & 0xFF) |
				(priority << ISU_SHIFT_PRIORITY);
		if (detection == INTC_LEVEL)
		{
			value |= ISU_LEVEL_TRIGGERED_BIT;
		}
		if (edge_type == INTC_ACTIVE_HIGH)
		{
			value |= ISU_POSITIVE_POLARITY_BIT;
		}
#if defined (OPIC_BIG_ENDIAN)
		opic_swap_endianness(&value);
#endif  // OPIC_BIG_ENDIAN
		out_u32((uint32_t*)
				(((char*)user_data) +
						(OPIC_ISU_BASE
								+ (INTC_SRC_N_VECT_PRIORITY_REGISTER
										+ (cause * ISU_BLOCK_SIZE)))),
										value);
	}
}

intc_t* opic_intc_create(
		mem_pool_info_t * const pool,
		const uint8_t * const base_address)
{
	intc_t * intc = NULL;
	kernel_intc_t * const intc_device = (kernel_intc_t*)mem_alloc(pool, sizeof(kernel_intc_t));
	if (intc_device)
	{
		intc_device->get_cause = opic_get;
		intc_device->ack_cause = opic_ack;
		intc_device->enable_cause = opic_enable;
		intc_device->mask_cause = opic_mask;
		intc_device->setup_cause = opic_setup;
		intc_device->user_data = base_address;
		intc = intc_create(pool, intc_device);

		uint32_t value = 0xC0;
#if defined (OPIC_BIG_ENDIAN)
		opic_swap_endianness(&value);
#endif  // OPIC_BIG_ENDIAN

		out_u32((uint32_t*)
				(((char*)base_address) +
						(OPIC_PROC_BASE
								+ (TASK_PRIORITY_REGISTER_N))), 0);

		out_u32((uint32_t*)
				(((char*)base_address) +
						(PROC_INIT_REGISTER)), value);
	}
	else
	{
		intc = NULL;
	}
	return intc;
}
