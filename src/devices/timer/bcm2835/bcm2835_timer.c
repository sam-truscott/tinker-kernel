/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "bcm2835_timer.h"
#include "tinker_api_time.h"

#define CONTROL_OFFSET 0
#define CLOCK_OFFSET 4
#define CLOCK_TIMER_COMPARE_0 0x0c
#define CLOCK_TIMER_COMPARE_1 0x10
#define CLOCK_TIMER_COMPARE_2 0x14
#define CLOCK_TIMER_COMPARE_3 0x18

typedef struct
{
	bool_t enabled;
	const tinker_time_t * alarm_time;
	uint8_t instance;
	void * base;
} bcm2835_timer_usr_data_t;

static void bcm2835_timer_setup(
		const timer_param_t const usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back)
{
	(void)call_back;
	if (usr_data)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)usr_data;
		*((tinker_time_t*)data->alarm_time) = *timeout;
		data->enabled = true;
		uint8_t offset;
		switch (data->instance)
		{
		case 0: offset = CLOCK_TIMER_COMPARE_0; break;
		case 1: offset = CLOCK_TIMER_COMPARE_1; break;
		case 2: offset = CLOCK_TIMER_COMPARE_2; break;
		case 3: offset = CLOCK_TIMER_COMPARE_3; break;
		default: offset = 0; break;
		}
		if (offset != 0)
		{
			out_u32((uint32_t*)(((uint8_t*)data->base) + offset), tinker_timer_get_microseconds(timeout));
		}
	}
}

static void bcm2835_timer_cancel(const timer_param_t const usr_data)
{
	if (usr_data)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)usr_data;
		if (data)
		{
			*((uint32_t*)usr_data) = 0;
		}
	}
}

void bcm2835_get_timer(mem_pool_info_t * const pool, timer_t * const timer, void * const base, const uint8_t instance)
{
	if (pool && timer)
	{
		timer->timer_setup = bcm2835_timer_setup;
		timer->timer_cancel = bcm2835_timer_cancel;
		timer->usr_data = (timer_param_t)mem_alloc(pool, sizeof(bcm2835_timer_usr_data_t));
		if (timer->usr_data)
		{
			timer->usr_data_size = sizeof(bcm2835_timer_usr_data_t);
			((bcm2835_timer_usr_data_t*)timer)->enabled = false;
			((bcm2835_timer_usr_data_t*)timer)->alarm_time = &TINKER_ZERO_TIME;
			((bcm2835_timer_usr_data_t*)timer)->instance = instance;
			((bcm2835_timer_usr_data_t*)timer)->base = base;
		}
		else
		{
			timer->usr_data = NO_TIMER_PARAM;
			timer->usr_data_size = 0;
		}
	}
}

bool_t bcm2835_has_timer_expired(const timer_t * const timer)
{
	bool_t fired = false;
	if (timer && timer->usr_data)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)timer->usr_data;
		const uint32_t control = in_u32((uint32_t*)(((uint8_t*)data->base + CONTROL_OFFSET)));
		if (control & (1 << data->instance))
		{
			fired = true;
		}
	}
	return fired;
}

void bcm2835_reset_timer(const timer_t * const timer)
{
	if (timer && timer->usr_data)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)timer->usr_data;
		const uint32_t control = in_u32((uint32_t*)(((uint8_t*)data->base + CONTROL_OFFSET)));
		out_u32((uint32_t*)(((uint8_t*)data->base + CONTROL_OFFSET)), control | (1 << data->instance));
	}
}

static uint64_t bcm2835_get_time(void * const user_data)
{
	volatile uint64_t * const timer = (uint64_t*)((uint8_t*)user_data + CLOCK_OFFSET);
	return ((*timer) * 1000);
}

clock_device_t * bcm2835_get_clock(void * const base_address, mem_pool_info_t * const pool)
{
	clock_device_t * const clock = mem_alloc(pool, sizeof(clock_device_t));
	if (clock)
	{
		clock->user_data = base_address;
		clock->get_time = bcm2835_get_time;
	}
	return clock;
}
