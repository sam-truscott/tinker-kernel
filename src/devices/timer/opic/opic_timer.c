/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "opic_timer.h"
#include "sos_api_errors.h"
#include "devices/intc/opic/opic_private.h"

#define TIMER_DISABLED 0x80000000

/**
 * Endian swap routine
 * @param var The variable to swap
 */
static void opic_swap_endianness(uint32_t* var);

static error_t __opic_timer_write_register(
		const void * usr_data,
		const uint32_t reg,
		const uint32_t value);

static void opic_tmr_timer_setup(
		const void * const usr_data,
		const __time_t timeout,
		__timer_callback * const call_back);

static void opic_tmr_timer_cancel(const void * const usr_data);

void opic_tmr_get_timer(uint32_t * base_address, __timer_t * timer)
{
	if ( base_address && timer )
	{
		timer->timer_setup = opic_tmr_timer_setup;
		timer->timer_cancel = opic_tmr_timer_cancel;
		timer->usr_data = (void*)base_address;
		timer->usr_data_size = (uint32_t)sizeof(uint32_t*);
	}
}

void opic_tmr_timer_setup(
		const void * const usr_data,
		const __time_t timeout,
		__timer_callback * const call_back)
{
	if ( usr_data && call_back )
	{
		__opic_timer_write_register(
				usr_data,
				TMR_N_VECTOR_PRIORITY_REGISTER,
				/* flags */
				(ISU_POSITIVE_POLARITY_BIT)
				 /* priority */
				 | (1 << 16)
				 /* port */
				 | 1);

		__opic_timer_write_register(
				usr_data,
				TMR_N_BASE_COUNT_REGISTER,
				TIMER_DISABLED);

		__opic_timer_write_register(
				usr_data,
				TMR_N_BASE_COUNT_REGISTER,
				/* TIMER_TICKS */timeout.seconds); /* TODO need to work this one out */
	}
}

void opic_tmr_timer_cancel(const void * const usr_data)
{
	__opic_timer_write_register(
			usr_data,
			TMR_N_BASE_COUNT_REGISTER,
			TIMER_DISABLED);
}

error_t __opic_timer_write_register(
		const void * usr_data,
		const uint32_t reg,
		const uint32_t value)
{
	error_t err = NO_ERROR;
	uint32_t new_value = value;

	uint32_t * base_addr = (uint32_t*)usr_data;

	switch( reg )
	{
		case TMR_N_CURRENT_COUNT_REGISTER:
		case TMR_N_BASE_COUNT_REGISTER:
		case TMR_N_VECTOR_PRIORITY_REGISTER:
		case TMR_N_DEST_REGISTER:
		case TMR_FREQ_REPORT_REGISTER:
			break;
		default:
			err = DEVICE_REGISTER_INVALID;
			break;
	}

#if defined(OPIC_BIG_ENDIAN)
	opic_swap_endianness(&new_value);
#endif

	if ( err == NO_ERROR )
	{
		uint32_t* p_dst = (uint32_t*)(base_addr + reg);
		*p_dst = new_value;
	}

	return err;
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
