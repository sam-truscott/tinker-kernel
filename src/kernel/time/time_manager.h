/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef CLOCK_MANAGER_H_
#define CLOCK_MANAGER_H_

#include "api/tinker_api_time.h"
#include "kernel/time/time.h"
#include "kernel/memory/memory_manager.h"

typedef struct time_manager_t time_manager_t;

/**
 * Initialise the time manager
 */
time_manager_t * time_initialise(mem_pool_info_t * const pool);

/**
 * Set the device to be used as the system clock
 * @param device The device to use as the system clock
 */
void time_set_system_clock(time_manager_t * const tm, clock_device_t * const device);

/**
 * Get the current time of the system
 * @return The time since power-up
 */
void time_get_system_time(time_manager_t * const tm, tinker_time_t * const time);

#endif /* CLOCK_MANAGER_H_ */
