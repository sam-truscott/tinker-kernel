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

#include "device_manager.h"

#include "../utils/collections/bounded_array.h"
#include "../memory/memory_manager.h"
#include "../kernel_initialise.h"

typedef struct
{
	__kernel_device_t * device;
	bool is_used;
} device_entry_t;

BOUNDED_ARRAY_SPEC(static, device_map_t, device_entry_t, __MAX_DEVICES)
BOUNDED_ARRAY_BODY(static, device_map_t, device_entry_t, __MAX_DEVICES)

static device_map_t * __kernel_devices = NULL;

void __devm_initialise(void)
{
	__process_t * kernel_process = __kernel_get_process();

	/* allocate space for the array */
	__kernel_devices = (device_map_t*)__mem_alloc(
			kernel_process->memory_pool,
			sizeof(device_map_t));

	/* initialise the the map from the kernel heap only*/
	device_map_t_initialise(__kernel_devices, kernel_process->memory_pool);
}

uint32_t __devm_install_device(__kernel_device_t * device)
{
	uint32_t new_device_id = 1;
	bool found = false;

	for ( ; new_device_id < __MAX_DEVICES ; new_device_id++ )
	{
		device_entry_t entry;
		entry = device_map_t_get(__kernel_devices, new_device_id);
		if ( !entry.is_used )
		{
			found = true;
			break;
		}
	}

	if ( found )
	{
		new_device_id = __devm_install_device_id(new_device_id, device);
	}
	return new_device_id;
}

uint32_t __devm_install_device_id(uint32_t device_id, __kernel_device_t * device)
{
	device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( !entry.is_used )
	{
		entry.is_used = true;
		entry.device = device;
		device_map_t_set(__kernel_devices, device_id, entry);
	} else {
		device_id = 0;
	}

	return device_id;
}

error_t __devm_uninstall_device(uint32_t device_id)
{
	error_t ret = NO_ERROR;

	device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used )
	{
		entry.is_used = false;
		device_map_t_set(__kernel_devices, device_id, entry);
	} else {
		ret = DEVICE_ID_INVALID;
	}

	return ret;
}

error_t __devm_initialise_device(uint32_t device_id, void * param, const uint32_t param_size)
{
	error_t err = NO_ERROR;

	const device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used && entry.device != NULL )
	{
		if ( entry.device->initialise )
		{
			entry.device->initialise(entry.device->user_data, param, param_size);
		}
		else
		{
			err = DEVICE_OPERATION_NOT_SUPPORTED;
		}
	} else {
		err = DEVICE_ID_INVALID;
	}

	return err;
}

error_t __devm_control(uint32_t device_id, uint32_t code)
{
	error_t err = NO_ERROR;

	const device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used && entry.device != NULL )
	{
		if ( entry.device->control)
		{
			entry.device->control(entry.device->user_data, code);
		}
		else
		{
			err = DEVICE_OPERATION_NOT_SUPPORTED;
		}
	} else {
		err = DEVICE_ID_INVALID;
	}

	return err;
}

error_t __devm_write_register(uint32_t device_id, uint32_t reg, uint32_t value)
{
	error_t err = NO_ERROR;

	const device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used && entry.device != NULL )
	{
		if ( entry.device->write_register )
		{
			entry.device->write_register(entry.device->user_data, reg, value);
		}
		else
		{
			err = DEVICE_OPERATION_NOT_SUPPORTED;
		}
	} else {
		err = DEVICE_ID_INVALID;
	}

	return err;
}

error_t __devm_read_register(uint32_t device_id, uint32_t reg, uint32_t * value)
{
	error_t err = NO_ERROR;

	const device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used && entry.device != NULL )
	{
		if ( entry.device->read_register )
		{
			entry.device->read_register(entry.device->user_data, reg, value);
		}
		else
		{
			err = DEVICE_OPERATION_NOT_SUPPORTED;
		}
	}
	else
	{
		err = DEVICE_ID_INVALID;
	}

	return err;
}

error_t __devm_write_buffer(uint32_t device_id, uint32_t dst, void * buffer, uint32_t size)
{
	error_t err = NO_ERROR;

	const device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used && entry.device != NULL )
	{
		if ( entry.device->write_buffer )
		{
			entry.device->write_buffer(entry.device->user_data, dst, buffer, size);
		}
		else
		{
			err = DEVICE_OPERATION_NOT_SUPPORTED;
		}
	}
	else
	{
		err = DEVICE_ID_INVALID;
	}

	return err;
}

error_t __devm_read_buffer(uint32_t device_id, uint32_t src, void * buffer, uint32_t size)
{
	error_t err = NO_ERROR;

	const device_entry_t entry = device_map_t_get(__kernel_devices, device_id);
	if ( entry.is_used && entry.device != NULL )
	{
		if ( entry.device->read_buffer )
		{
			entry.device->read_buffer(entry.device->user_data, src, buffer, size);
		}
		else
		{
			err = DEVICE_OPERATION_NOT_SUPPORTED;
		}
	}
	else
	{
		err = DEVICE_ID_INVALID;
	}

	return err;
}
