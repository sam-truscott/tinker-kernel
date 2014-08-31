/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arm_vec.h"

#define KSTACKEXC   0x4000

#define KEXP_TOPSWI \
	uint32_t sp,lr,r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,apsr; \
	asm("mov %[ps], sp" : [ps]"=r" (sp)); \
	asm("mov sp, %[ps]" : : [ps]"i" (KSTACKEXC)); \
	asm("mov %[ps], lr" : [ps]"=r" (lr)); \
	asm("mov %[ps], r0" : [ps]"=r" (r0)); \
	asm("mov %[ps], r1" : [ps]"=r" (r1)); \
	asm("mov %[ps], r2" : [ps]"=r" (r2)); \
	asm("mov %[ps], r3" : [ps]"=r" (r3)); \
	asm("mov %[ps], r4" : [ps]"=r" (r4)); \
	asm("mov %[ps], r5" : [ps]"=r" (r5)); \
	asm("mov %[ps], r6" : [ps]"=r" (r6)); \
	asm("mov %[ps], r7" : [ps]"=r" (r7)); \
	asm("mov %[ps], r8" : [ps]"=r" (r8)); \
	asm("mov %[ps], r9" : [ps]"=r" (r9)); \
	asm("mov %[ps], r10" : [ps]"=r" (r10)); \
	asm("mov %[ps], r11" : [ps]"=r" (r11)); \
	asm("mov %[ps], r12" : [ps]"=r" (r12)); \
	asm("mov %[ps], apsr" : [ps]"=r" (apsr));

#define KEXP_TOP3 \
	uint32_t sp,lr,r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,apsr; \
	asm("mov %[ps], sp" : [ps]"=r" (sp)); \
	asm("mov sp, %[ps]" : : [ps]"i" (KSTACKEXC)); \
	asm("sub lr, lr, #4"); \
	asm("mov %[ps], lr" : [ps]"=r" (lr)); \
	asm("mov %[ps], r0" : [ps]"=r" (r0)); \
	asm("mov %[ps], r1" : [ps]"=r" (r1)); \
	asm("mov %[ps], r2" : [ps]"=r" (r2)); \
	asm("mov %[ps], r3" : [ps]"=r" (r3)); \
	asm("mov %[ps], r4" : [ps]"=r" (r4)); \
	asm("mov %[ps], r5" : [ps]"=r" (r5)); \
	asm("mov %[ps], r6" : [ps]"=r" (r6)); \
	asm("mov %[ps], r7" : [ps]"=r" (r7)); \
	asm("mov %[ps], r8" : [ps]"=r" (r8)); \
	asm("mov %[ps], r9" : [ps]"=r" (r9)); \
	asm("mov %[ps], r10" : [ps]"=r" (r10)); \
	asm("mov %[ps], r11" : [ps]"=r" (r11)); \
	asm("mov %[ps], r12" : [ps]"=r" (r12)); \
	asm("mov %[ps], apsr" : [ps]"=r" (apsr));

#define KEXP_BOT3 \
	/* asm("pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}"); */ \
	asm("LDM sp!, {pc}^")

static arm_vec_handler_t * vector_table[8];

static void __attribute__((naked)) arm_vector_reset()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_RESET](VECTOR_RESET, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_undefined()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_UNDEFINED](VECTOR_UNDEFINED, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_prefetch_abort()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_PRETECH_ABORT](VECTOR_PRETECH_ABORT, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_data_abort()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_DATA_ABORT](VECTOR_DATA_ABORT, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_reserved()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_RESERVED](VECTOR_RESERVED, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_irq()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_IRQ](VECTOR_IRQ, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_fiq()
{
	KEXP_TOP3;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_FIQ](VECTOR_FIQ, context);
	KEXP_BOT3;
}

static void __attribute__((naked)) arm_vector_system_call()
{
	KEXP_TOPSWI;
	tgt_context_t context;
	context.sp = sp;
	context = vector_table[VECTOR_SYSTEM_CALL](VECTOR_SYSTEM_CALL, context);
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
