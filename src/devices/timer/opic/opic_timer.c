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

#include "opic_timer.h"
#include "devices/interrupt_controller/opic/opic_intc.h"

#define OPIC_BIG_ENDIAN

#define TIMER_DISABLED 0x80000000

/**
 * Endian swap routine
 * @param var The variable to swap
 */
static void opic_swap_endianness(uint32_t* var);

/*
static error_t __opic_timer_read_register(
		const void * base_addr,
		const UINT32 reg,
		UINT32 *value);
*/

static error_t __opic_timer_write_register(
		const void * usr_data,
		const uint32_t reg,
		const uint32_t value);

static void opic_tmr_timer_setup(void * usr_data,__time_t timeout, __timer_callback * call_back);

static void opic_tmr_timer_cancel(void * usr_data);

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

void opic_tmr_timer_setup(void * usr_data, __time_t timeout, __timer_callback * call_back)
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

void opic_tmr_timer_cancel(void * usr_data)
{
	__opic_timer_write_register(
			usr_data,
			TMR_N_BASE_COUNT_REGISTER,
			TIMER_DISABLED);
}

/*
error_t __opic_timer_read_register(
		const void * base_addr,
		const UINT32 reg,
		UINT32 *value)
{
	error_t err = NO_ERROR;

	UINT32 * base_addr = (UINT32*)usr_data;

	if ( base_addr == NULL )
	{
		err = DEVICE_ID_INVALID;
	}

	switch( reg )
	{
		case TMR_N_CURRENT_COUNT_REGISTER:
		case TMR_N_BASE_COUNT_REGISTER:
		case TMR_N_VECTOR_PRIORITY_REGISTER:
		case TMR_N_DEST_REGISTER:
			break;
		default:
			err = DEVICE_REGISTER_INVALID;
			break;
	}

	if ( err == NO_ERROR )
	{
		UINT32 *v = (UINT32*)(base_addr + reg);
		*value = *v;

#ifdef OPIC_BIG_ENDIAN
		opic_swap_endianness(value);
#endif
	}

	return err;
}
*/

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

#ifdef OPIC_BIG_ENDIAN
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
