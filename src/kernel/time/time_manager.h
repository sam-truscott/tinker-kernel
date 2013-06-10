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

#ifndef CLOCK_MANAGER_H_
#define CLOCK_MANAGER_H_

#include "../kernel_types.h"

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
