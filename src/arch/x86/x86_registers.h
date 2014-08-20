/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef X86_REGISTERS_H_
#define X86_REGISTERS_H_

#include "arch/tgt_types.h"

uint32_t x86_get_cr0(void);

static inline void x86_set_fs(uint16_t seg)
{
	asm volatile("movw %0,%%fs" : : "rm" (seg));
}

static inline void x86_set_gs(uint16_t seg)
{
	asm volatile("movw %0,%%gs" : : "rm" (seg));
}

#endif /* X86_REGISTERS_H_ */
