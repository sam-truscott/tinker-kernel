/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "intc.h"

#include "kernel/kernel_assert.h"
#include "kernel/utils/collections/hashed_map.h"

typedef struct
{
	__intc_device_type type;
	union
	{
		const __intc_t * child;
		const __kernel_device_t * device;
		const __object_pipe_t * pipe;
	} devices;
} intc_internal_t;

HASH_MAP_TYPE_T(__isr_map_t)
HASH_MAP_INTERNAL_TYPE_T(__isr_map_t, uint32_t, intc_internal_t*, __MAX_ISRS, 16)
HASH_MAP_SPEC_CREATE(static, __isr_map_t)
HASH_MAP_SPEC_INITALISE(static, __isr_map_t)
HASH_MAP_SPEC_DELETE(static, __isr_map_t)
HASH_MAP_SPEC_PUT(static, __isr_map_t, uint32_t, intc_internal_t*)
HASH_MAP_SPEC_GET(static, __isr_map_t, uint32_t, intc_internal_t*)
HASH_MAP_SPEC_CONTAINS_KEY(static, __isr_map_t, uint32_t)
HASH_FUNCS_VALUE(__isr_map_t, uint32_t)
HASH_MAP_BODY_CREATE(static, __isr_map_t)
HASH_MAP_BODY_INITALISE(static, __isr_map_t, __MAX_ISRS, 16)
HASH_MAP_BODY_DELETE(static, __isr_map_t, __MAX_ISRS, 16)
HASH_MAP_BODY_PUT(static, __isr_map_t, uint32_t, intc_internal_t*, __MAX_ISRS, 16)
HASH_MAP_BODY_GET(static, __isr_map_t, uint32_t, intc_internal_t*, 16)
HASH_MAP_BODY_CONTAINS_KEY(static, __isr_map_t, uint32_t, 16)

typedef struct __intc_t
{
	__mem_pool_info_t * pool;
	__isr_map_t * isr_map;
	__kernel_intc_t * kernel_intc;
} __intc_it_t;

__intc_t * __intc_create(__mem_pool_info_t * const pool, __kernel_intc_t * const kernel_intc)
{
	__intc_t * const intc = __mem_alloc(pool, sizeof(__intc_t));
	if (intc)
	{
		intc->kernel_intc = kernel_intc;
		intc->pool = pool;
		intc->isr_map = __isr_map_t_create(
			__hash_basic_integer,
			__hash_equal_integer,
			true,
			pool);
	}
	return intc;
}

void __intc_delete(__intc_t * const intc)
{
	if (intc && intc->isr_map)
	{
		__isr_map_t_delete(intc->isr_map);
	}
}

void __intc_add_child(__intc_t * const intc, const uint32_t cause, const __intc_t * child)
{
	if (intc)
	{
		intc_internal_t * const internal = __mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = CHILD_INTC;
			internal->devices.child = child;
			const bool_t put_ok = __isr_map_t_put(intc->isr_map, cause, internal);
			__kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void __intc_add_device(__intc_t * const intc, const uint32_t cause, const __kernel_device_t * device)
{
	if (intc)
	{
		intc_internal_t * const internal = __mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = DEVICE_INTC;
			internal->devices.device = device;
			const bool_t put_ok = __isr_map_t_put(intc->isr_map, cause, internal);
			__kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void __intc_add_pipe(__intc_t * const intc, const uint32_t cause, const __object_pipe_t * pipe)
{
	if (intc)
	{
		intc_internal_t * const internal = __mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = PIPE_INTC;
			internal->devices.pipe = pipe;
			const bool_t put_ok = __isr_map_t_put(intc->isr_map, cause, internal);
			__kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

error_t __intc_handle(const __intc_t * const intc)
{
	error_t ret = NO_ERROR;
	if (intc)
	{
		uint32_t cause = 0;
		if (intc->kernel_intc->get_cause(&cause, intc->kernel_intc->user_data))
		{
			if (__isr_map_t_contains_key(intc->isr_map, cause))
			{
				intc_internal_t * dev = NULL;
				if (__isr_map_t_get(intc->isr_map, cause, &dev))
				{
					switch(dev->type)
					{
					case DEVICE_INTC:
						dev->devices.device->isr(dev->devices.device->user_data, cause);
						break;
					case CHILD_INTC:
						__intc_handle(dev->devices.child);
						break;
					case PIPE_INTC:
						ret = __obj_pipe_send_message(
								(__object_pipe_t*)dev->devices.pipe,
								0, /* no need, block is false */
								PIPE_TX_SEND_AVAILABLE,
								(void*)&cause, sizeof(cause),
								false);
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

error_t __intc_enable(__intc_t * const intc, const uint32_t cause)
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

error_t __intc_mask(__intc_t * const intc, const uint32_t cause)
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
