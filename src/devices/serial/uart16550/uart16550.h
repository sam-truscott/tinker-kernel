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

#ifndef UART16550_H_
#define UART16550_H_

#include "kernel/kernel_types.h"

/**
 * List of valid UART 16550 registers
 */
typedef enum
{
	REG_0_RX_TX_LS = 0,
	REG_1_ISR_MS = 1,
	REG_2_IIR = 2,
	REG_3_LCR = 3,
	REG_4_MCR = 4,
	REG_5_LSR = 5,
	REG_6_MSR = 6,
	REG_7_SCRATCH = 7
} uart_16550_register_t;

/**
 * Get the kernel device instance for a given base address
 * @param base_address The base address of the device
 * @param device The new kernel device
 */
void uart16550_get_device(void * base_address, __kernel_device_t * device);

#endif /* UART16550_H_ */
