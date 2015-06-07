/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "opic_timer.h"
#include "tinker_api_errors.h"
#include "devices/intc/opic/opic_private.h"

#define TIMER_DISABLED 0x80000000

/**
 * Endian swap routine
 * @param var The variable to swap
 */
static void opic_swap_endianness(uint32_t* var);

static error_t opic_timer_write_register(
		const void * usr_data,
		const uint32_t reg,
		const uint32_t value);

static void opic_tmr_timer_setup(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back,
		void * const param);

typedef struct opic_timer_t
{
	void * base_address;
	void * param;
} opic_timer_t;

static void opic_tmr_timer_cancel(timer_param_t usr_data);

void opic_tmr_get_timer(mem_pool_info_t * const pool, uint32_t * base_address, timer_t * timer)
{
	if (base_address && timer && pool)
	{
		timer->timer_setup = opic_tmr_timer_setup;
		timer->timer_cancel = opic_tmr_timer_cancel;
		timer->usr_data = mem_alloc(pool, sizeof(opic_timer_t));
		if (timer->usr_data)
		{
			opic_timer_t * const opic_data = (opic_timer_t*)timer->usr_data;
			opic_data->base_address = base_address;
			opic_data->param = NULL;
			timer->usr_data_size = sizeof(opic_timer_t);
		}
	}
}

void opic_tmr_timer_setup(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back,
		void * param)
{
	if (usr_data && call_back)
	{
		((opic_timer_t*)usr_data)->param = param;
		opic_timer_write_register(
				((opic_timer_t*)usr_data)->base_address,
				TMR_N_VECTOR_PRIORITY_REGISTER,
				/* flags */
				(ISU_POSITIVE_POLARITY_BIT)
				 /* priority */
				 | (1 << 16)
				 /* port */
				 | 1);

		opic_timer_write_register(
				((opic_timer_t*)usr_data)->base_address,
				TMR_N_BASE_COUNT_REGISTER,
				TIMER_DISABLED);

		opic_timer_write_register(
				((opic_timer_t*)usr_data)->base_address,
				TMR_N_BASE_COUNT_REGISTER,
				/* TIMER_TICKS */timeout->seconds); /* TODO need to work this one out */
	}
}

void opic_tmr_timer_cancel(timer_param_t usr_data)
{
	opic_timer_write_register(
			((opic_timer_t*)&usr_data)->base_address,
			TMR_N_BASE_COUNT_REGISTER,
			TIMER_DISABLED);
}

error_t opic_timer_write_register(
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
	const uint32_t copy = *var;
	*var = ((copy & 0xFF) << 24) |
			((copy & 0xFF00) << 8) |
			((copy & 0xFF0000) >> 8) |
			((copy & 0xFF000000) >> 24);
}
