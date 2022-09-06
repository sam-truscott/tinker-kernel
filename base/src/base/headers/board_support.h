/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef BOARD_SUPPORT_H_
#define BOARD_SUPPORT_H_

#include "config.h"
#include "tinker_api.h"
#include "tinker_api_types.h"
#include "tinker_api_errors.h"
#include "interrupts/interrupt_manager.h"
#include "time/alarm_manager.h"

/**
 * Initialise the Board Support package. This should
 * initialise the both target (processor) and the
 * board / environment
 */
void bsp_initialise(void) BOOT_CODE;

void bsp_setup(
		interrupt_controller_t * const controller,
		time_manager_t * const tm,
		alarm_manager_t * const am,
		process_t * const kernel_process);

/**
 * Call from the kernel to tell the BSP to activate the
 * required timer to perform the next context switch
 */
void bsp_enable_schedule_timer(void);

mem_t bsp_get_usable_memory_start(void);

mem_t bsp_get_usable_memory_end(void);

void bsp_write_debug_char(const char c);

#endif /* BOARD_SUPPORT_H_ */
