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

#include "kernel_assert.h"
#include "console/print_out.h"
#include "devices/kernel_device.h"

#define CONTROL_OFFSET 0
#define CLOCK_OFFSET 4
#define CLOCK_TIMER_COMPARE_0 0x0c
#define CLOCK_TIMER_COMPARE_1 0x10
#define CLOCK_TIMER_COMPARE_2 0x14
#define CLOCK_TIMER_COMPARE_3 0x18

typedef struct
{
	timer_callback * callback;
	uint8_t instance;
	void * base;
	void * param;
} bcm2835_timer_usr_data_t;

static void bcm2835_timer_setup(
		const timer_param_t usr_data,
		const tinker_time_t * const timeout,
		timer_callback * const call_back,
		void * const param)
{
	if (is_debug_enabled(TIMER))
	{
		debug_print(TIMER, "BCM2835: Setting up timer with user data %x, timeout s %d.%d, callback %x\n",
				usr_data, timeout->seconds, timeout->nanoseconds, call_back);
	}
	if (usr_data)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)usr_data;
		uint8_t offset;
		data->callback = call_back;
		data->param = param;
		switch (data->instance)
		{
			case 0: offset = CLOCK_TIMER_COMPARE_0; break;
			case 1: offset = CLOCK_TIMER_COMPARE_1; break;
			case 2: offset = CLOCK_TIMER_COMPARE_2; break;
			case 3: offset = CLOCK_TIMER_COMPARE_3; break;
			default: offset = 0; break;
		}
		kernel_assert("BCM2835 timer has an invalid offset", offset != 0);
		if (offset != 0)
		{
			const uint32_t old_control = in_u32((uint32_t*)(((uint8_t*)data->base) + CONTROL_OFFSET));
			if (old_control & (1 << data->instance))
			{
				const uint32_t new_control = (1 << data->instance);
				out_u32((uint32_t*)(((uint8_t*)data->base + CONTROL_OFFSET)), new_control);
				if (is_debug_enabled(TIMER))
				{
					debug_print(TIMER, "BCM2835: Clearing status for instance %d, base %x, offset %x, control %x -> %x\n",
							data->instance, data->base, CONTROL_OFFSET, old_control, new_control);
				}
			}
			const uint32_t timeAsUs = tinker_timer_get_microseconds(timeout);
#if defined (KERNEL_DEBUGGING)
			if (is_debug_enabled(TIMER))
			{
				const uint32_t current = in_u32((uint32_t *)(((uint8_t*)data->base + CLOCK_OFFSET)));
				debug_print(TIMER, "BCM2835: Setting up timer for instance %d, base %x, offset %x, value %d, current %d\n",
						data->instance,
						data->base,
						offset,
						timeAsUs,
						current);
			}
#endif
			out_u32((uint32_t*)(((uint8_t*)data->base) + offset), timeAsUs);
		}
	}
}

static void bcm2835_timer_cancel(const timer_param_t usr_data)
{
	if (usr_data)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)usr_data;
		if (data)
		{
			uint8_t offset;
			switch (data->instance)
			{
			case 0: offset = CLOCK_TIMER_COMPARE_0; break;
			case 1: offset = CLOCK_TIMER_COMPARE_1; break;
			case 2: offset = CLOCK_TIMER_COMPARE_2; break;
			case 3: offset = CLOCK_TIMER_COMPARE_3; break;
			default: offset = 0; break;
			}
			kernel_assert("BCM2835 timer cancel offset is invalid", offset != 0);
			if (offset != 0)
			{
				// read the control
				uint32_t control = in_u32((uint32_t*)(((uint8_t*)data->base + CONTROL_OFFSET)));
				if (control & (1 << data->instance))
				{
					// clear the bit in control
					out_u32((uint32_t*)(((uint8_t*)data->base + CONTROL_OFFSET)), (1 << data->instance));
					if (is_debug_enabled(TIMER))
					{
						debug_print(TIMER, "BCM2835: Cancelling status for instance %d, base %x, offset %x, control %x -> %x\n",
								data->instance, data->base, CONTROL_OFFSET, control, (1 << data->instance));
					}
				}
			}
			else
			{
				kernel_panic();
			}
		}
	}
}

static return_t bcm2835_timer_isr(tgt_context_t * const context, timer_param_t param)
{
	return_t result;
	bcm2835_timer_cancel(param);
	if (is_debug_enabled(TIMER))
	{
		debug_prints(TIMER, "BCM2835: ISR for timer\n");
	}
	if (param)
	{
		bcm2835_timer_usr_data_t * const data = (bcm2835_timer_usr_data_t*)param;
		if (data->callback)
		{
			if (is_debug_enabled(TIMER))
			{
				debug_print(TIMER, "BCM2835: Calling back to %x\n", data->callback);
			}
			data->callback(context, data->param);
			if (is_debug_enabled(TIMER))
			{
				debug_prints(TIMER, "BCM2835: Called back\n");
			}
			result = NO_ERROR;
		}
		else
		{
			result = TIMER_CALLBACK_MISSING;
		}
	}
	else
	{
		result = PARAMETERS_INVALID;
	}
	return result;
}

void bcm2835_get_timer(mem_pool_info_t * const pool, timer_t * const timer, void * const base, const uint8_t instance)
{
	if (pool && timer)
	{
		timer->timer_setup = &bcm2835_timer_setup;
		timer->timer_cancel = &bcm2835_timer_cancel;
		timer->timer_isr = &bcm2835_timer_isr;
		timer->usr_data = (timer_param_t)mem_alloc(pool, sizeof(bcm2835_timer_usr_data_t));
		if (timer->usr_data)
		{
			timer->usr_data_size = sizeof(bcm2835_timer_usr_data_t);
			((bcm2835_timer_usr_data_t*)timer->usr_data)->instance = instance;
			uint32_t vbase = 0;
			kernel_device_map_memory((uint32_t)base, 0x1000, MMU_DEVICE_MEMORY, &vbase);
			((bcm2835_timer_usr_data_t*)timer->usr_data)->base = (void*)vbase;
			// get the control for the timer
			const uint32_t control = in_u32((uint32_t*)(((uint8_t*)vbase + CONTROL_OFFSET)));
			// if the control is set for this instance; write a 1 and wipe/reset it
			if (control & (1 << instance))
			{
				out_u32((uint32_t*)(((uint8_t*)vbase + CONTROL_OFFSET)), (1 << instance));
			}
		}
		else
		{
			timer->usr_data = NO_TIMER_PARAM;
			timer->usr_data_size = 0;
		}
	}
}

static uint64_t bcm2835_get_time(void * const user_data)
{
	//volatile uint64_t * const timer = (volatile uint64_t*)((uint8_t*)user_data + CLOCK_OFFSET);
	uint32_t timer_upper1 = in_u32((uint32_t *)(((uint8_t*)user_data + CLOCK_OFFSET + CLOCK_OFFSET)));
	uint32_t timer_lower = in_u32((uint32_t *)(((uint8_t*)user_data + CLOCK_OFFSET)));
	uint32_t timer_upper2 = in_u32((uint32_t *)(((uint8_t*)user_data + CLOCK_OFFSET + CLOCK_OFFSET)));
	while (timer_upper1 != timer_upper2)
	{
		timer_upper1 = in_u32((uint32_t *)((uint8_t*)user_data + CLOCK_OFFSET + CLOCK_OFFSET));
		timer_lower = in_u32((uint32_t *)((uint8_t*)user_data + CLOCK_OFFSET));
		timer_upper2 = in_u32((uint32_t *)((uint8_t*)user_data + CLOCK_OFFSET + CLOCK_OFFSET));
	}
	uint64_t timer = timer_upper1;
	timer = timer << 32;
	timer += timer_lower;
	// 1Mhz timer
	return (timer * 1000);
}

clock_device_t * bcm2835_get_clock(void * const base_address, mem_pool_info_t * const pool)
{
	clock_device_t * const clock = mem_alloc(pool, sizeof(clock_device_t));
	if (clock)
	{
		uint32_t vbase = 0;
		kernel_device_map_memory((uint32_t)base_address, 0x1000, MMU_DEVICE_MEMORY, &vbase);
		clock->user_data = (void*)vbase;
		clock->get_time = bcm2835_get_time;
	}
	return clock;
}
