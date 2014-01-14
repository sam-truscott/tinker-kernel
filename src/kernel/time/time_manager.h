/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef CLOCK_MANAGER_H_
#define CLOCK_MANAGER_H_

#include "api/sos_api_time.h"
#include "kernel/time/time.h"

/**
 * Initialise the time manager
 */
void __time_initialise(void);

/**
 * Set the device to be used as the system clock
 * @param device The device to use as the system clock
 */
void __time_set_system_clock(__clock_device_t * const device);

/**
 * Get the current time of the system
 * @return The time since power-up
 */
void __time_get_system_time(sos_time_t * const time);

#endif /* CLOCK_MANAGER_H_ */
