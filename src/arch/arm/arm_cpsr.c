/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arm_cpsr.h"
#include "kernel/utils/util_memcpy.h"

static uint32_t arm_get_cpsr(void)
{
	uint32_t r;
	asm("mrs %[ps], cpsr" : [ps]"=r" (r));
	return r;
}

static void arm_set_cpsr(const uint32_t r)
{
    asm("msr cpsr, %[ps]" : : [ps]"r" (r));
}

psr_mode_t arm_get_psr_mode(void)
{
	return arm_get_cpsr() & 0x1F;
}

void arm_set_psr_mode(const psr_mode_t psr_mode)
{
	arm_set_cpsr((arm_get_cpsr() & 0x1F) | psr_mode);
}

void arm_get_psr_mode_name(const psr_mode_t psr_mode, char * const buffer, const uint32_t size)
{
	switch (psr_mode)
	{
		case PSR_MODE_USER:
			util_memcpy(buffer, "USER", size);
			break;
		case PSR_MODE_FIQ:
			util_memcpy(buffer, "FIQ", size);
			break;
		case PSR_MODE_IRQ:
			util_memcpy(buffer, "IRQ", size);
			break;
		case PSR_MODE_SUPERVISOR:
			util_memcpy(buffer, "SUPERVISOR", size);
			break;
		case PSR_MODE_ABORT:
			util_memcpy(buffer, "ABORT", size);
			break;
		case PSR_MODE_UNDEFINED:
			util_memcpy(buffer, "UNDEFINED", size);
			break;
		case PSR_MODE_SYSTEM:
			util_memcpy(buffer, "SYSTEM", size);
			break;
		default:
			break;
	}
}
