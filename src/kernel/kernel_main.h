/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_

#include "kernel_types.h"

/**
 * Start up the Kernel
 */
void __kernel_main(void);

bool __kernel_is_first_run(void);

void __kernel_first_run_ok(void);

#endif /* KERNEL_MAIN_H_ */
