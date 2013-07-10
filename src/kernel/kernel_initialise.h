/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KERNEL_INITIALISE_H_
#define KERNEL_INITIALISE_H_

#include "kernel_types.h"

#include "kernel/process/process.h"

/**
 * Initialise the internals of the kernel.
 * This should only be done once the BSP has been initialised
 */
void __kernel_initialise(void);

__process_t * __kernel_get_process(void);

__thread_t * __kernel_get_idle_thread(void);

__thread_t * __kernel_get_scheduler_thread(void);

#endif /* KERNEL_INITIALISE_H_ */
