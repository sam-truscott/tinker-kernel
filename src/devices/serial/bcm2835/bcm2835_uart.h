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

#include "arch/tgt_types.h"
#include "kernel/devices/kernel_device.h"

void early_uart_init(void);

void early_uart_putc(const char c);

void bcm2835_uart_get_device(
		kernel_device_t * const device);

#endif // #ifndef UART_H_
