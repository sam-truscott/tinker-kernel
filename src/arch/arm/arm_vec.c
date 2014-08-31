/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arm_vec.h"

#define KEXP_TOPSWI \
	uint32_t context; \
	asm("stmfd sp!,{r0-r12,lr}"); \
	asm("mrs r0, spsr"); \
	asm("push {r0}"); \
	asm("mov %[ps], sp" : [ps]"=r" (context));

#define KEXP_TOP3 \
	uint32_t context; \
	asm("sub lr, lr, #4"); \
	asm("stmfd sp!,{r0-r12,lr}"); \
	asm("mrs r0, spsr"); \
	asm("push {r0}"); \
	asm("mov %[ps], sp" : [ps]"=r" (context));

#define KEXP_BOT3 \
	asm("pop {r0, r3}"); /* apsr */\
	asm("msr SPSR_cxsf, r0"); \
	asm("ldm sp!, {r0-r12,pc}^")

static arm_vec_handler_t * vector_table[8];

static void __attribute__((naked)) arm_vector_reset()
{
	KEXP_TOP3;
	vector_table[VECTOR_RESET](VECTOR_RESET, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_undefined()
{
	KEXP_TOP3;
	vector_table[VECTOR_UNDEFINED](VECTOR_UNDEFINED, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_prefetch_abort()
{
	KEXP_TOP3;
	vector_table[VECTOR_PRETECH_ABORT](VECTOR_PRETECH_ABORT, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_data_abort()
{
	KEXP_TOP3;
	vector_table[VECTOR_DATA_ABORT](VECTOR_DATA_ABORT, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_reserved()
{
	KEXP_TOP3;
	vector_table[VECTOR_RESERVED](VECTOR_RESERVED, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_irq()
{
	KEXP_TOP3;
	vector_table[VECTOR_IRQ](VECTOR_IRQ, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_fiq()
{
	KEXP_TOP3;
	vector_table[VECTOR_FIQ](VECTOR_FIQ, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_system_call()
{
	KEXP_TOPSWI;
	vector_table[VECTOR_SYSTEM_CALL](VECTOR_SYSTEM_CALL, context);
	KEXP_BOT3;
}

void arm_vec_install(arm_vec_t vector, arm_vec_handler_t * const handler)
{
	vector_table[vector] = handler;
	uint32_t * v = (uint32_t*)0x0;
	switch(vector)
	{
	case VECTOR_RESET:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_reset - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_UNDEFINED:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_undefined - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_PRETECH_ABORT:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_prefetch_abort - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_DATA_ABORT:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_data_abort - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_RESERVED:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_reserved - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_IRQ:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_irq - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_FIQ:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_fiq - (8 + (4 * vector))) >> 2);
		break;
	case VECTOR_SYSTEM_CALL:
		v[vector] = 0xEA000000 | (((uint32_t)arm_vector_system_call - (8 + (4 * vector))) >> 2);
		break;
	}
}
