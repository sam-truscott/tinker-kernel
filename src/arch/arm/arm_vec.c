/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arm_vec.h"
#include "kernel/console/print_out.h"

#define KEXP_TOPSWI \
	uint32_t context; \
	asm("stmfd sp!,{r0-r12,lr}");	/* store all the registers */ \
	asm("mrs r0, spsr"); 			/* get the spsr */ \
	asm("mov r1, sp"); 				/* get the new sp */ \
	asm("push {r0, r1}"); 			/* store the spsr and sp */ \
	asm("mov %[ps], sp" : [ps]"=r" (context)); /* move the sp into context var */

#define KEXP_TOP3 \
	uint32_t context; \
	asm("sub lr, lr, #4"); 			/* update return addr */ \
	asm("stmfd sp!,{r0-r12,lr}");	/* store all the registers */ \
	asm("mrs r0, spsr"); 			/* get the spsr */ \
	asm("mov r1, sp"); 				/* get the new sp */ \
	asm("push {r0, r1}"); 			/* store the spsr and sp */ \
	asm("mov %[ps], sp" : [ps]"=r" (context)); /* move the sp into context var */

#define KEXP_BOT3 \
	asm("nop"); \
	asm("mrs r3, cpsr"); 			/* backup cpsr */ \
	asm("msr cpsr, #0x12");			/* enter irq mode */ \
	asm("ldr sp, =__ivtse");			/* setup irq stack */ \
	asm("msr cpsr, r3");			/* restore old cpsr */ \
	asm("pop {r0, r1}"); 			/* get the spsr back */ \
	asm("msr SPSR_cxsf, r0"); 		/* restore spsr */ \
	asm("ldm sp!, {r0-r12,pc}^")	/* return! */

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

#define OFFSET_FOR_PREFETCH 8
#define ALIGNMENT_SHIFT 2
#define BRANCH_INSTRUCTION 0xEA000000
#define GET_BRANCH_FOR_VECTOR(v, i) \
	((((uint32_t)v - (i*4) - (OFFSET_FOR_PREFETCH)) >> ALIGNMENT_SHIFT) | BRANCH_INSTRUCTION)

void arm_vec_install(arm_vec_t vector, arm_vec_handler_t * const handler)
{
	printp_out("Installing vector %d to route to %x\n", vector, handler);
	vector_table[vector] = handler;
	volatile uint32_t * const vector_root = (uint32_t*)0x0;
	switch(vector)
	{
	case VECTOR_RESET:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_reset, vector);
		break;
	case VECTOR_UNDEFINED:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_undefined, vector);
		break;
	case VECTOR_PRETECH_ABORT:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_prefetch_abort, vector);
		break;
	case VECTOR_DATA_ABORT:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_data_abort, vector);
		break;
	case VECTOR_RESERVED:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_reserved, vector);
		break;
	case VECTOR_IRQ:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_irq, vector);
		break;
	case VECTOR_FIQ:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_fiq, vector);
		break;
	case VECTOR_SYSTEM_CALL:
		vector_root[vector] = GET_BRANCH_FOR_VECTOR(&arm_vector_system_call, vector);
		break;
	}
}
