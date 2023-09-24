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

typedef struct intc_t
{
	mem_pool_info_t * pool;
	intc_internal_t * isrs[MAX_ISRS];
	kernel_intc_t * kernel_intc;
} intc_it_t;

intc_t * intc_create(mem_pool_info_t * const pool, kernel_intc_t * const kernel_intc)
{
	intc_t * const intc = mem_alloc(pool, sizeof(intc_t));
	if (intc)
	{
		intc->kernel_intc = kernel_intc;
		intc->pool = pool;
		for (int i = 0 ; i < MAX_ISRS ; i++)
		{
			intc->isrs[i] = NULL;
		}
	}
	return intc;
}

void intc_delete(intc_t * const intc)
{
	if (intc)
	{
		mem_pool_info_t * const pool = intc->pool;
		if (pool != NULL)
		{
			for (int i = 0 ; i < MAX_ISRS ; i++)
			{
				if (intc->isrs[i] != NULL)
				{
					mem_free(pool, intc->isrs[i]);
				}
			}
			mem_free(pool, intc);
		}
	}
}

void intc_add_child(intc_t * const intc, const uint32_t cause, const intc_t * const child)
{
	if (cause > MAX_ISRS)
	{
		kernel_assert("add interrupt out of range", false);
	}
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = CHILD_INTC;
			internal->devices.child = child;
			const bool_t put_ok = intc->isrs[cause] == NULL;
			if (put_ok)
			{
				intc->isrs[cause] = internal;
			}
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void intc_add_device(intc_t * const intc, const uint32_t cause, const kernel_device_t * const device)
{
	if (cause > MAX_ISRS)
	{
		kernel_assert("add interrupt out of range", false);
	}
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = DEVICE_INTC;
			internal->devices.device = device;
			const bool_t put_ok = intc->isrs[cause] == NULL;
			if (put_ok)
			{
				intc->isrs[cause] = internal;
			}
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void intc_add_pipe(intc_t * const intc, const uint32_t cause, const object_pipe_t * const pipe)
{
	if (cause > MAX_ISRS)
	{
		kernel_assert("add interrupt out of range", false);
	}
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = PIPE_INTC;
			internal->devices.pipe = pipe;
			const bool_t put_ok = intc->isrs[cause] == NULL;
			if (put_ok)
			{
				intc->isrs[cause] = internal;
			}
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

void intc_add_timer(intc_t * const intc, const uint32_t cause, const timer_t * const timer)
{
	if (cause > MAX_ISRS)
	{
		kernel_assert("add interrupt out of range", false);
	}
	if (intc)
	{
		intc_internal_t * const internal = mem_alloc(intc->pool, sizeof(intc_internal_t));
		if (internal)
		{
			internal->type = TIMER_INTC;
			internal->devices.timer = timer;
			const bool_t put_ok = intc->isrs[cause] == NULL;
			if (put_ok)
			{
				intc->isrs[cause] = internal;
			}
			kernel_assert("failed to add interrupt device to controller", put_ok);
		}
	}
}

return_t intc_handle(const intc_t * const intc, tgt_context_t * const context)
{
	return_t ret = NO_ERROR;
	if (intc)
	{
		uint32_t cause = 0;
		if (intc->kernel_intc->get_cause(&cause, intc->kernel_intc->user_data))
		{
			if (cause > MAX_ISRS)
			{
				ret = UNKNOWN_INTERRUPT_CAUSE;
			}
			else
			{
				intc_internal_t * const dev = intc->isrs[cause];
				if (dev != NULL)
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
					default:
						ret = UNKNOWN_INTERRUPT_CAUSE;
						break;
					}
				}
				else
				{
					ret = UNKNOWN_INTERRUPT_CAUSE;
				}
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

return_t intc_setup(
		intc_t * const intc,
		const uint32_t cause,
		const intc_priority_t priority,
		const intc_detection_type detection,
		const intc_active_type edge_type)
{
	return_t ret = NO_ERROR;
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

return_t intc_enable(intc_t * const intc, const uint32_t cause)
{
	return_t ret = NO_ERROR;
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

return_t intc_mask(intc_t * const intc, const uint32_t cause)
{
	return_t ret = NO_ERROR;
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
