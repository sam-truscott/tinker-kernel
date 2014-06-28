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

/**
 * Initialise the time manager
 */
void time_initialise(void);

/**
 * Set the device to be used as the system clock
 * @param device The device to use as the system clock
 */
void time_set_system_clock(clock_device_t * const device);

/**
 * Get the current time of the system
 * @return The time since power-up
 */
void time_get_system_time(tinker_time_t * const time);

#endif /* CLOCK_MANAGER_H_ */
