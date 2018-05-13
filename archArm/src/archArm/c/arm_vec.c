/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arm_vec.h"
#include "console/print_out.h"
#pragma GCC optimize ("-O0")

#define SWITCH_TO_SYSTEM_MODE \
	asm volatile("mrs r0, cpsr"); \
	asm volatile("mov r2, r0"); \
	asm volatile("mov r1, #0xFFFFFF80"); \
	asm volatile("and r0, r0, r1"); \
	asm volatile("mov r1, #0xDF"); \
	asm volatile("orr r0, r0, r1"); \
	asm volatile("msr cpsr, r0");

#define SWITCH_BACK \
	asm volatile("msr cpsr, r2");

#define FIX_STACK_ALIGNMENT \
	asm volatile("push {r1}");

#define UNFIX_STACK_ALIGNMENT \
	asm volatile("pop {r1}");

#define EXCEPTION_START_COMMON \
	uint32_t context; \
	asm volatile("stmfd sp!,{r0-r12,lr}");				/* store all the registers */ \
	asm volatile("mrs r0, spsr"); 						/* get the spsr */ \
	asm volatile("push {r0}"); 							/* store the spsr */ \
	SWITCH_TO_SYSTEM_MODE								/* switch to system mode so we can get r13(sp), r14(lr) */ \
	asm volatile("mov r3, sp");							/* get user stack pointer */ \
	asm volatile("mov r4, lr");							/* get user link pointer */ \
	SWITCH_BACK \
	asm volatile("push {r3, r4}");						/* store sp, lr */ \
	FIX_STACK_ALIGNMENT \
	asm volatile("mov %[ps], sp" : [ps]"=r" (context)); /* move the sp into context var */ \

#define EXCEPTION_START_ERROR \
	uint32_t context; \
	asm volatile("stmfd sp!,{r0-r12,lr}");				/* store all the registers */ \
	asm volatile("mrs r0, spsr"); 						/* get the spsr */ \
	asm volatile("push {r0}");							/* switch to system mode so we can get r13(sp), r14(lr) */ \
	asm volatile("push {r13, r14}");					/* store sp, lr */ \
	FIX_STACK_ALIGNMENT \
	asm volatile("mov %[ps], sp" : [ps]"=r" (context)); /* move the sp into context var */ \

#define EXCEPTION_START_SYSCALL \
	EXCEPTION_START_COMMON

#define EXCEPTION_START_VECTOR \
	/*asm volatile("sub lr, lr, #4");*/ 					/* update return addr */ \
	EXCEPTION_START_ERROR

#define EXCEPTION_START_ISR \
	asm volatile("sub lr, lr, #4"); 					/* update return addr */ \
	EXCEPTION_START_COMMON

#define EXCEPTION_END \
	asm volatile("nop"); \
	UNFIX_STACK_ALIGNMENT \
	asm volatile("pop {r3, r4}");						/* get the sp and pc back */ \
	SWITCH_TO_SYSTEM_MODE \
	asm volatile("mov sp, r3");							/* restore the sp */ \
	asm volatile("mov lr, r4");							/* restore the sp */ \
	SWITCH_BACK \
	asm volatile("pop {r0}");							/* get the spsr back */ \
	asm volatile("msr SPSR_cxsf, r0"); 					/* restore spsr */ \
	asm volatile("ldm sp!, {r0-r12,pc}^");				/* return! */

static arm_vec_handler_t * vector_table[8];

static void __attribute__((naked,used)) arm_vector_reset()
{
	EXCEPTION_START_VECTOR;
	vector_table[VECTOR_RESET](VECTOR_RESET, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_undefined()
{
	EXCEPTION_START_VECTOR;
	vector_table[VECTOR_UNDEFINED](VECTOR_UNDEFINED, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_prefetch_abort()
{
	EXCEPTION_START_VECTOR;
	vector_table[VECTOR_PRETECH_ABORT](VECTOR_PRETECH_ABORT, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_data_abort()
{
	EXCEPTION_START_VECTOR;
	vector_table[VECTOR_DATA_ABORT](VECTOR_DATA_ABORT, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_reserved()
{
	EXCEPTION_START_VECTOR;
	vector_table[VECTOR_RESERVED](VECTOR_RESERVED, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_irq()
{
	EXCEPTION_START_ISR;
	vector_table[VECTOR_IRQ](VECTOR_IRQ, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_fiq()
{
	EXCEPTION_START_ISR;
	vector_table[VECTOR_FIQ](VECTOR_FIQ, context);
	EXCEPTION_END;
}

static void __attribute__((naked,used)) arm_vector_system_call()
{
	EXCEPTION_START_SYSCALL;
	vector_table[VECTOR_SYSTEM_CALL](VECTOR_SYSTEM_CALL, context);
	EXCEPTION_END;
}

#define OFFSET_FOR_PREFETCH 8
#define ALIGNMENT_SHIFT 2
#define BRANCH_INSTRUCTION 0xEA000000
#define GET_BRANCH_FOR_VECTOR(v, i) \
	((((uint32_t)v - (i*4) - (OFFSET_FOR_PREFETCH)) >> ALIGNMENT_SHIFT) | BRANCH_INSTRUCTION)

void arm_vec_install(arm_vec_t vector, arm_vec_handler_t * const handler)
{
	volatile uint32_t * const vector_root = (uint32_t*)0x0;
	vector_table[vector] = handler;
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
