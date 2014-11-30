/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_KERNEL_IN_H_
#define KERNEL_KERNEL_IN_H_

#include "arch/tgt_types.h"

void kernel_in_initialise(void);

void kernel_in_write(char * const buffer, const uint16_t size);


#endif /* KERNEL_KERNEL_IN_H_ */
