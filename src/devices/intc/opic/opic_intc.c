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

static void __opic_swap_endianness(uint32_t* var)
{
	const uint32_t copy = *var;
	*var = ((copy & 0xFF) << 24) |
			((copy & 0xFF00) << 8) |
			((copy & 0xFF0000) >> 8) |
			((copy & 0xFF000000) >> 24);
}

static bool_t __opic_get(
		uint32_t * const cause,
		const void * const user_data)
{
	uint32_t * addr = (uint32_t*)
			(((char*)user_data) +
					(OPIC_PROC_BASE
							+ (INTERRUPT_ACK_REGISTER_N)));
	*cause = __in_u32(addr);
#if defined (OPIC_BIG_ENDIAN)
		__opic_swap_endianness(cause);
#endif  // OPIC_BIG_ENDIAN
	return (*cause) > 0;
}

static void __opic_ack(
		const uint32_t cause,
		const void * const user_data)
{
	uint32_t cause_copy = cause;
#if defined (OPIC_BIG_ENDIAN)
		__opic_swap_endianness(&cause_copy);
#endif  // OPIC_BIG_ENDIAN
	uint32_t * addr = (uint32_t*)
			(((char*)user_data) +
					(OPIC_PROC_BASE
							+ (END_OF_INTERRUPT_REGISTER_N)));
	__out_u32(addr, 0/*cause_copy*/);
}

static void __opic_mask(
		const uint32_t cause,
		const void * const user_data)
{
	// TODO: ISU register setup
	if (cause && user_data){}
}

static void __opic_enable(
		const uint32_t cause,
		const void * const user_data)
{
	if (cause && user_data)
	{
		uint32_t value = (cause & 0xFF) |
				(1 << ISU_SHIFT_PRIORITY) |
				ISU_LEVEL_TRIGGERED_BIT |
				ISU_POSITIVE_POLARITY_BIT;
#if defined (OPIC_BIG_ENDIAN)
		__opic_swap_endianness(&value);
#endif  // OPIC_BIG_ENDIAN
		__out_u32((uint32_t*)
				(((char*)user_data) +
						(OPIC_ISU_BASE
								+ (INTC_SRC_N_VECT_PRIORITY_REGISTER
										+ (cause * ISU_BLOCK_SIZE)))),
										value);
		__out_u32((uint32_t*)
				(((char*)user_data) +
						(OPIC_ISU_BASE
								+ (INTC_SRC_N_DEST_REGISTER
										+ (cause * ISU_BLOCK_SIZE)))),
										0xFFFFFFFF); //FIXME TODO Find out why 1 doesn't work
	}
}

__intc_t* __opic_intc_create(
		__mem_pool_info_t * const pool,
		const uint8_t * const base_address)
{
	__intc_t * intc = NULL;
	__kernel_intc_t * const intc_device = (__kernel_intc_t*)__mem_alloc(pool, sizeof(__kernel_intc_t));
	if (intc_device)
	{
		intc_device->get_cause = __opic_get;
		intc_device->ack_cause = __opic_ack;
		intc_device->enable_cause = __opic_enable;
		intc_device->mask_cause = __opic_mask;
		intc_device->user_data = base_address;
		intc = __intc_create(pool, intc_device);

		uint32_t value = 0xC0;
#if defined (OPIC_BIG_ENDIAN)
		__opic_swap_endianness(&value);
#endif  // OPIC_BIG_ENDIAN

		__out_u32((uint32_t*)
				(((char*)base_address) +
						(OPIC_PROC_BASE
								+ (TASK_PRIORITY_REGISTER_N))), 0);

		__out_u32((uint32_t*)
				(((char*)base_address) +
						(PROC_INIT_REGISTER)), value);
	}
	else
	{
		intc = NULL;
	}
	return intc;
}
