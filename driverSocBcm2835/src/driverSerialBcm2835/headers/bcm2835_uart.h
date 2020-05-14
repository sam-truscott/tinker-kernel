/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

/* uart.h - UART initialization & communication */

#ifndef UART_H_
#define UART_H_

#include "tgt_types.h"
#include "devices/kernel_device.h"

void early_uart_init(const uint8_t * const base_address);

void early_uart_putc(const char c);

void early_uart_put(const char * c);

void bcm2835_uart_get_device(
		const uint8_t * const base_address,
		kernel_device_t * const device,
		const char * const name);

#endif // #ifndef UART_H_
