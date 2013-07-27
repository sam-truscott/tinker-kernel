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

#include "kernel/kernel_types.h"
#include "kernel/time/time.h"

/**
 * Initialise the time manager
 */
void __time_initialise(void);

/**
 * Set the device to be used as the system clock
 * @param device The device to use as the system clock
 */
void __time_set_system_clock(__clock_device_t * device);

/**
 * Get the current time of the system
 * @return The time since power-up
 */
__time_t __time_get_system_time(void);

/**
 * Sleep for a given period of time
 * # NOTE - This is a tight loop. Processes
 * should use the syscall to enter a sleep
 * state for a given period of time before
 * an alarm is raised.
 * @param period The period to sleep for
 */
void __time_sleep(__time_t period);

#endif /* CLOCK_MANAGER_H_ */
