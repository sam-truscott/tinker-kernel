/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef ARM_PSR_H_
#define ARM_PSR_H_

#include "arch/tgt_types.h"

typedef enum psr_mode
{
	PSR_MODE_USER = 0x10,
	PSR_MODE_FIQ = 0x11,
	PSR_MODE_IRQ = 0x12,
	PSR_MODE_SUPERVISOR = 0x13,
	PSR_MODE_ABORT = 0x17,
	PSR_MODE_UNDEFINED = 0x1B,
	PSR_MODE_SYSTEM = 0x1F
} psr_mode_t;

uint32_t arm_get_cpsr(void);

void arm_set_cpsr(const uint32_t r);

void arm_enable_irq(void);

void arm_disable_irq(void);

void arm_disable_fiq(void);

psr_mode_t arm_get_psr_mode(void);

#endif /* ARM_PSR_H_ */
