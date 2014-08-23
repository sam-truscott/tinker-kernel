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

#include "config.h"
#include "arch/tgt_types.h"

uint32_t x86_get_cr0(void) BOOT_CODE;

#define DELAY asm volatile("outb %%al,%0" : : "dN" (0x80))

static inline void x86_set_fs(const uint16_t seg) BOOT_CODE;
static inline void x86_set_gs(const uint16_t seg) BOOT_CODE;
static inline void x86_outb(const uint8_t v, const uint16_t port) BOOT_CODE;

static inline void x86_set_fs(const uint16_t seg)
{
	asm volatile("movw %0,%%fs" : : "rm" (seg));
}

static inline void x86_set_gs(const uint16_t seg)
{
	asm volatile("movw %0,%%gs" : : "rm" (seg));
}

static inline void x86_outb(const uint8_t v, const uint16_t port)
{
	asm volatile("outb %0,%1" : : "a" (v), "dN" (port));
}

#endif /* X86_REGISTERS_H_ */
