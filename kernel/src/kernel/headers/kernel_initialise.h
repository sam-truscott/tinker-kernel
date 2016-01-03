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

#include "interrupts/interrupt_manager.h"
#include "process/process_list.h"
#include "process/process.h"

/**
 * Initialise the internals of the kernel.
 * This should only be done once the BSP has been initialised
 */
void kernel_initialise(void);

process_t * kernel_get_process(void);

registry_t * kernel_get_registry(void);

proc_list_t * kernel_get_proc_list(void); //FIXME remove

#endif /* KERNEL_INITIALISE_H_ */
