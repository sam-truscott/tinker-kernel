/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef ARM_VEC_H_
#define ARM_VEC_H_

#include "tgt_types.h"

typedef enum arm_vec
{
	VECTOR_RESET = 0x00,
	VECTOR_UNDEFINED = 0x01,
	VECTOR_SYSTEM_CALL = 0x02,
	VECTOR_PRETECH_ABORT = 0x03,
	VECTOR_DATA_ABORT = 0x04,
	VECTOR_RESERVED = 0x05,
	VECTOR_IRQ = 0x06,
	VECTOR_FIQ = 0x07
} arm_vec_t;

/**
 * The structure of the saved interrupt vector context
 */
#define ARM_CONTEXT_GPR 13
#define ARM_FP_REGISTER 11

#pragma pack(push,1)
typedef struct tgt_context_t
{
	uint32_t sp;
	uint32_t lr;
	uint32_t cpsr;
    uint32_t gpr[ARM_CONTEXT_GPR];
    uint32_t pc;
} tgt_context_internal_t;
#pragma pack(pop)

typedef void arm_vec_handler_t(arm_vec_t type, uint32_t contextp);

void arm_vec_install(arm_vec_t vector, arm_vec_handler_t * const handler);


#endif /* ARM_VEC_H_ */
