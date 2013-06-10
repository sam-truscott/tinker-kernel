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

#ifndef BOARD_SUPPORT_H_
#define BOARD_SUPPORT_H_

#include "sos3.h"
#include "kernel/scheduler/scheduler.h"

/**
 * Initialise the Board Support package. This should
 * initialise the both target (processor) and the
 * board / environment
 */
void __bsp_initialise(void);

void __bsp_setup(void);

/**
 * Call from the kernel to tell the BSP to activate the
 * required timer to perform the next context switch
 */
void __bsp_enable_schedule_timer(void);

uint32_t __bsp_get_usable_memory_start();

uint32_t __bsp_get_usable_memory_end();

void __bsp_write_debug_char(const char c);

char __bsp_read_debug_char(void);

#endif /* BOARD_SUPPORT_H_ */
