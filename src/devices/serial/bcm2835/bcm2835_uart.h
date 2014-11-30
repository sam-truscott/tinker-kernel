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

/*
 * Initialize UART0.
 */
void bcm2835_uart_init();

/*
 * Transmit a byte via UART0.
 * uint8_t Byte: byte to send.
 */
void bcm2835_uart_putc(uint8_t byte);

/*
 * print a string to the UART one character at a time
 * const char *str: 0-terminated string
 */
void bcm2835_uart_puts(const char * str);

/*
 * Receive a byte via UART0.
 *
 * Returns:
 * uint8_t: byte received.
 */
uint8_t bcm2835_uart_getc();

void bcm2835_uart_get_device(
		kernel_device_t * const device);

#endif // #ifndef UART_H_
