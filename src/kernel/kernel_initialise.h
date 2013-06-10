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

#ifndef KERNEL_INITIALISE_H_
#define KERNEL_INITIALISE_H_

#include "kernel_types.h"

/**
 * Initialise the internals of the kernel.
 * This should only be done once the BSP has been initialised
 */
void __kernel_initialise(void);

__process_t * __kernel_get_process(void);

__thread_t * __kernel_get_idle_thread(void);

__thread_t * __kernel_get_scheduler_thread(void);

#endif /* KERNEL_INITIALISE_H_ */
