/*************************************************************************
 *
 * SOS 3 Source Code
 * __________________
 *
 *  [2012] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef POWERPC32_H_
#define POWERPC32_H_

#include "arch/ivt.h"
#include "arch/tgt.h"

#define PPC_CONTEXT_GPR 30
#define PPC_CONTEXT_FPR	32

/*
 * Enumeration of all the MSR, Machine State Register,
 * flags that can be enabled
 */
typedef enum
{
	/**
	 * Power Management Enable
	 */
	MSR_FLAG_POW = 	0x00040000,
	MSR_FLAG_TGPR = 0x00020000,
	MSR_FLAG_ILE =	0x00010000,
	/**
	 * External Interrupts
	 */
	MSR_FLAG_EE = 	0x00008000,
	/**
	 * Priv: 0 = Supervisor, 1 = Problem
	 */
	MSR_FLAG_PR =	0x00004000,
	/**
	 * Floating point support
	 */
	MSR_FLAG_FP =	0x00002000,
	/**
	 * Machine Check Enable: 1 = Enabled
	 */
	MSR_FLAG_ME = 	0x00001000,
	MSR_FLAG_FE0 = 	0x00000800,
	MSR_FLAG_SE = 	0x00000400,
	MSR_FLAG_BE = 	0x00000200,
	MSR_FLAG_FE1 = 	0x00000100,
	/**
	 * Interupt Prefix: RAM = 0
	 */
	MSR_FLAG_IP = 	0x00000040,
	MSR_FLAG_IR = 	0x00000020,
	MSR_FLAG_DR = 	0x00000010,
	/**
	 * Recoverable Interrupts: Yes = 1
	 */
	MSR_FLAG_RI = 	0x00000002,
	/**
	 * Little Endian
	 */
	MSR_FLAG_LE = 	0x00000001
} ppc32_msr_flags;

typedef enum __ppc32_interrupt_vector
{
	__ppc32_vector_system_reset = 1,
	__ppc32_vector_machine_check = 2,
	__ppc32_vector_data_storage = 3,
	__ppc32_vector_instruction_storage = 4,
	__ppc32_vector_external_interrupt = 5,
	__ppc32_vector_alignment = 6,
	__ppc32_vector_program_error = 7,
	__ppc32_vector_floating_point_unavailable = 8,
	__ppc32_vector_decrementer = 9,
	__ppc32_vector_syscall = 12,
	__ppc32_vector_trace = 13,
	__ppc32_vector_floating_point_assist = 14
} __ppc32_interrupt_vector_t;

EXTERN uint32_t __ivt_system_reset_interrupt;
EXTERN uint32_t __ivt_machine_check_interrupt;
EXTERN uint32_t __ivt_data_storage_interrupt;
EXTERN uint32_t __ivt_inst_storage_interrupt;
EXTERN uint32_t __ivt_external_interrupt;
EXTERN uint32_t __ivt_alignment_interrupt;
EXTERN uint32_t __ivt_program_interrupt;
EXTERN uint32_t __ivt_fp_unavailable;
EXTERN uint32_t __ivt_decrementer_interrupt;
EXTERN uint32_t __ivt_syscall_interrupt;
EXTERN uint32_t __ivt_trace_interrupt;
EXTERN uint32_t __ivt_fp_assist_interrupt;

/**
 * The structure of the saved interrupt vector context
 */
#pragma pack(push,1)
typedef struct __ppc32_ivt_struct
{
	uint32_t sp;
	uint32_t restore_lr;
	uint32_t link_area_1;
	uint32_t link_area_2;
	uint32_t gpr_0;
	uint32_t gpr_2_31[PPC_CONTEXT_GPR];
	uint32_t srr0;
	uint32_t srr1;
	uint32_t xer;
	uint32_t cr;
	uint32_t ctr;
	uint64_t fpr[PPC_CONTEXT_FPR]; /* FIXME need to be moved to the end */
	uint32_t lr;
	uint32_t fp;
} __ppc32_ivt_struct_t;
#pragma pack(pop)

/**
 * Type declaration for interrupt service routines
 * @param context The saved context from the interrupt vector
 */
typedef void(__ppc_isr)(uint32_t vector, __ppc32_ivt_struct_t * context, bool fp_enabled);

/**
 * Initialise the basic powerpc interrupt services
 */
void __ppc_isr_initialise(void);

/**
 * Attach an Interrupt Service Routine to a Vector
 */
void __ppc_isr_attach(const uint32_t vector, __ppc_isr * isr);

/**
 * Get the Interrupt Service Routine for a vector
 */
__ppc_isr * __ppc_isr_get_isr(const uint32_t vector);

/**
 * Detach an Interrupt Service Routine from a Vector
 */
void __ppc_isr_detach(const uint32_t vector);

/**
 * Set the Machine State Register
 */
void __ppc_set_msr(uint32_t msr);

/**
 * Get the Machine State Register
 */
uint32_t __ppc_get_msr(void);

uint32_t __ppc_get_srr0(void);
uint32_t __ppc_get_srr1(void);

/**
 * Get the combined lower and upper time base
 * @return 64 bit value for the upper and lower timebase register value
 */
uint64_t __ppc_get_tbr(void);

/**
 * Set the lower and upper bits of the time base register
 * @param tbu The upper time base DWORD
 * @param tbl The lower time base DWORD
 */
void __ppc_set_tbr(uint32_t tbu, uint32_t tbl);

/**
 * For a given clock speed and ticks for the TBR, calculate
 * the number of nanoseconds per timebase register tick
 * @param clock_hz The clock speed of the processor
 * @param ticks_per_clock The number of timebase register 'ticks' per clock cycle
 * @return The number of nanoseconds per timebase register 'tick'
 */
uint32_t __ppc_get_ns_per_tb_tick(uint64_t clock_hz, uint32_t ticks_per_clock);

/**
 * Set the value of the decrementer
 */
void __ppc_set_decrementer(uint32_t v);

/**
 * The place where the IVT routines will end up
 * once they've saved the context
 */
void __ppc_isr_handler(const uint32_t vector, void * registers, bool fp_enabled);

/**
 * Setup the powerpc instruction breakpoint register IDAR
 */
void __ppc_set_instruction_breakpoint(uint32_t location);

/**
 * Setup the powerpc data breakpoint register DDAR
 */
void __ppc_set_data_breakpoint(uint32_t location);

uint32_t __ppc_get_dsisr(void);

#endif /* POWERPC32_H_ */
