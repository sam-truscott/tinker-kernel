/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "interrupts/intc.h"

#include "kernel_assert.h"
#include "utils/collections/hashed_map.h"

typedef struct
{
	intc_device_type type;
	union
	{
		const intc_t * child;
		const kernel_device_t * device;
		const object_pipe_t * pipe;
		const timer_t * timer;
	} devices;
} intc_internal_t;

HASH_MAP_TYPE_T(isr_map_t)
HASH_MAP_INTERNAL_TYPE_T(isr_map_t, uint32_t, intc_internal_t*, MAX_ISRS, 16)
HASH_MAP_SPEC_CREATE(static, isr_map_t)
HASH_MAP_SPEC_INITALISE(static, isr_map_t)
HASH_MAP_SPEC_DELETE(static, isr_map_t)
HASH_MAP_SPEC_PUT(static, isr_map_t, uint32_t, intc_internal_t*)
HASH_MAP_SPEC_GET(static, isr_map_t, uint32_t, intc_internal_t*)
HASH_MAP_SPEC_CONTAINS_KEY(static, isr_map_t, uint32_t)
HASH_FUNCS_VALUE(isr_map_t, uint32_t)
HASH_MAP_BODY_CREATE(static, isr_map_t)
HASH_MAP_BODY_INITALISE(static, isr_map_t, MAX_ISRS, 16)
HASH_MAP_BODY_DELETE(static, isr_map_t, MAX_ISRS, 16)
HASH_MAP_BODY_PUT(static, isr_map_t, uint32_t, intc_internal_t*, MAX_ISRS, 16)
HASH_MAP_BODY_GET(static, isr_map_t, uint32_t, intc_internal_t*, 16)
HASH_MAP_BODY_CONTAINS_KEY(static, isr_map_t, uint32_t, 16)

typedef struct intc_t
{
	mem_pool_info_t * pool;
	isr_map_t * isr_map;
	kernel_intc_t * kernel_intc;
} intc_it_t;

intc_t * intc_create(mem_pool_info_t * const pool, kernel_intc_t * const kernel_intc)
{
	intc_t * const intc = mem_alloc(pool, sizeof(intc_t));
	if (intc)
	{
		intc->kernel_intc = kernel_intc;
		intc->pool = pool;
		intc->isr_map = isr_map_t_create(
			hash_basic_integer,
			hash_equal_integer,
			true,
			pool);
	}
	return intc;
}

void intc_delete(intc_t * const intc)
{
	if (intc && intc->isr_map)
	{
		isr_map_t_delete(intc->isr_map);
	}
}

void intc_add_child(intc_t * const intc, const uint32_t cause, const intc_t * const child)
{
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = CHILD_INTC;
			internal->devices.child = child;
			const bool_t put_ok = isr_map_t_put(intc->isr_map, cause, internal);
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void intc_add_device(intc_t * const intc, const uint32_t cause, const kernel_device_t * const device)
{
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = DEVICE_INTC;
			internal->devices.device = device;
			const bool_t put_ok = isr_map_t_put(intc->isr_map, cause, internal);
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void intc_add_pipe(intc_t * const intc, const uint32_t cause, const object_pipe_t * const pipe)
{
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = PIPE_INTC;
			internal->devices.pipe = pipe;
			const bool_t put_ok = isr_map_t_put(intc->isr_map, cause, internal);
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void intc_add_timer(intc_t * const intc, const uint32_t cause, const timer_t * const timer)
{
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = TIMER_INTC;
			internal->devices.timer = timer;
			const bool_t put_ok = isr_map_t_put(intc->isr_map, cause, internal);
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

error_t intc_handle(const intc_t * const intc, tgt_context_t * const context)
{
	error_t ret = NO_ERROR;
	if (intc)
	{
		uint32_t cause = 0;
		if (intc->kernel_intc->get_cause(&cause, intc->kernel_intc->user_data))
		{
			if (isr_map_t_contains_key(intc->isr_map, cause))
			{
				intc_internal_t * dev = NULL;
				if (isr_map_t_get(intc->isr_map, cause, &dev))
				{
					switch(dev->type)
					{
					case DEVICE_INTC:
						ret = dev->devices.device->isr(dev->devices.device->user_data, cause);
						break;
					case CHILD_INTC:
						ret = intc_handle(dev->devices.child, context);
						break;
					case PIPE_INTC:
						ret = obj_pipe_send_message(
								(object_pipe_t*)dev->devices.pipe,
								0, /* no need, block is false */
								PIPE_TX_SEND_AVAILABLE,
								(void*)&cause, sizeof(cause),
								false);
						break;
					case TIMER_INTC:
						if (dev->devices.timer && dev->devices.timer->timer_isr)
						{
							ret = dev->devices.timer->timer_isr(context, dev->devices.timer->usr_data);
						}
						else
						{
							ret = UNKNOWN_INTERRUPT_CAUSE;
						}
						break;
					case EMPTY_DEVICE:
						ret = UNKNOWN_INTERRUPT_CAUSE;
						break;
					}
				}
				else
				{
					ret = UNKNOWN_INTERRUPT_CAUSE;
				}
			}
			else
			{
				ret = UNKNOWN_INTERRUPT_CAUSE;
			}
			intc->kernel_intc->ack_cause(cause, intc->kernel_intc->user_data);
		}
		else
		{
			ret = UNKNOWN_INTERRUPT_CAUSE;
		}
	}
	else
	{
		ret = NO_ROOT_INTERRUPT_HANDLER;
	}
	return ret;
}

error_t intc_setup(
		intc_t * const intc,
		const uint32_t cause,
		const intc_priority_t priority,
		const intc_detection_type detection,
		const intc_active_type edge_type)
{
	error_t ret = NO_ERROR;
	if (intc)
	{
		intc->kernel_intc->setup_cause(
				cause, priority, detection, edge_type, intc->kernel_intc->user_data);
	}
	else
	{
		ret = NO_ROOT_INTERRUPT_HANDLER;
	}
	return ret;
}

error_t intc_enable(intc_t * const intc, const uint32_t cause)
{
	error_t ret = NO_ERROR;
	if (intc)
	{
		intc->kernel_intc->enable_cause(cause, intc->kernel_intc->user_data);
	}
	else
	{
		ret = NO_ROOT_INTERRUPT_HANDLER;
	}
	return ret;
}

error_t intc_mask(intc_t * const intc, const uint32_t cause)
{
	error_t ret = NO_ERROR;
	if (intc)
	{
		intc->kernel_intc->mask_cause(cause, intc->kernel_intc->user_data);
	}
	else
	{
		ret = NO_ROOT_INTERRUPT_HANDLER;
	}
	return ret;
}
