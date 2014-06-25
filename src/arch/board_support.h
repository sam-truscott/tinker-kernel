/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef BOARD_SUPPORT_H_
#define BOARD_SUPPORT_H_

#include "kernel/tinker.h"
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

void __bsp_check_timers_and_alarms(void);

uint32_t __bsp_get_usable_memory_start();

uint32_t __bsp_get_usable_memory_end();

void __bsp_write_debug_char(const char c);

char __bsp_read_debug_char(void);

#endif /* BOARD_SUPPORT_H_ */
