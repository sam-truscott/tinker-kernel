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
#include "api/tinker_api.h"
#include "api/tinker_api_types.h"
#include "api/tinker_api_errors.h"
#include "kernel/interrupts/interrupt_manager.h"

/**
 * Initialise the Board Support package. This should
 * initialise the both target (processor) and the
 * board / environment
 */
void bsp_initialise(void) BOOT_CODE;

void bsp_setup(interrupt_controller_t * const controller);

/**
 * Call from the kernel to tell the BSP to activate the
 * required timer to perform the next context switch
 */
void bsp_enable_schedule_timer(void);

void bsp_check_timers_and_alarms(const tgt_context_t * const context);

uint32_t bsp_get_usable_memory_start();

uint32_t bsp_get_usable_memory_end();

void bsp_write_debug_char(const char c);

#endif /* BOARD_SUPPORT_H_ */
