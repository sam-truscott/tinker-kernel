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

#ifndef DEVICE_MANAGER_H_
#define DEVICE_MANAGER_H_

#include "../kernel_types.h"

void __devm_initialise(void);

uint32_t __devm_install_device(__kernel_device_t * device);

uint32_t __devm_install_device_id(uint32_t device_id, __kernel_device_t * device);

error_t __devm_uninstall_device(uint32_t device_id);

error_t __devm_initialise_device(uint32_t device_id, void * param, const uint32_t param_size);

error_t __devm_control(uint32_t device_id, uint32_t code);

error_t __devm_write_register(uint32_t device_id, uint32_t reg, uint32_t value);

error_t __devm_read_register(uint32_t device_id, uint32_t reg, uint32_t * value);

error_t __devm_write_buffer(uint32_t device_id, uint32_t dst, void * buffer, uint32_t size);

error_t __devm_read_buffer(uint32_t device_id, uint32_t src, void * buffer, uint32_t size);

#endif /* DEVICE_MANAGER_H_ */
