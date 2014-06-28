/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KERNEL_INITIALISE_H_
#define KERNEL_INITIALISE_H_

#include "kernel/process/process.h"

/**
 * Initialise the internals of the kernel.
 * This should only be done once the BSP has been initialised
 */
void kernel_initialise(void);

process_t * kernel_get_process(void);

thread_t * kernel_get_idle_thread(void);

#endif /* KERNEL_INITIALISE_H_ */
