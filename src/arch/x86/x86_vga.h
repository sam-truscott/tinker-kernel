/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef X86_VGA_H_
#define X86_VGA_H_

#include "tgt_ints.h"

void x86_vga_initialise(void);

void x86_vga_putchar(char c);

void x86_vga_writestring(const char* const data, const uint32_t max);

#endif /* X86_VGA_H_ */
