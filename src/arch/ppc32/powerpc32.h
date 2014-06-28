/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
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

typedef enum ppc32_interrupt_vector
{
	ppc32_vector_system_reset = 1,
	ppc32_vector_machine_check = 2,
	ppc32_vector_data_storage = 3,
	ppc32_vector_instruction_storage = 4,
	ppc32_vector_external_interrupt = 5,
	ppc32_vector_alignment = 6,
	ppc32_vector_program_error = 7,
	ppc32_vector_floating_point_unavailable = 8,
	ppc32_vector_decrementer = 9,
	ppc32_vector_syscall = 12,
	ppc32_vector_trace = 13,
	ppc32_vector_floating_point_assist = 14
} ppc32_interrupt_vector_t;

extern uint32_t ivt_system_reset_interrupt;
extern uint32_t ivt_machine_check_interrupt;
extern uint32_t ivt_data_storage_interrupt;
extern uint32_t ivt_inst_storage_interrupt;
extern uint32_t ivt_external_interrupt;
extern uint32_t ivt_alignment_interrupt;
extern uint32_t ivt_program_interrupt;
extern uint32_t ivt_fp_unavailable;
extern uint32_t ivt_decrementer_interrupt;
extern uint32_t ivt_syscall_interrupt;
extern uint32_t ivt_trace_interrupt;
extern uint32_t ivt_fp_assist_interrupt;

/**
 * Type declaration for interrupt service routines
 * @param context The saved context from the interrupt vector
 */
typedef void(ppc_isr)(const uint32_t vector, tgt_context_t * const context);

/**
 * Initialise the basic powerpc interrupt services
 */
void ppc_isr_initialise(void);

/**
 * Attach an Interrupt Service Routine to a Vector
 */
void ppc_isr_attach(const uint32_t vector, ppc_isr * const isr);

/**
 * Get the Interrupt Service Routine for a vector
 */
ppc_isr * ppc_isr_get_isr(const uint32_t vector);

/**
 * Set the Machine State Register
 */
void ppc_set_msr(const uint32_t msr);

/**
 * Get the Machine State Register
 */
uint32_t ppc_get_msr(void);

uint32_t ppc_get_srr0(void);
uint32_t ppc_get_srr1(void);

/**
 * Get the combined lower and upper time base
 * @return 64 bit value for the upper and lower timebase register value
 */
uint64_t ppc_get_tbr(void);

/**
 * Set the lower and upper bits of the time base register
 * @param tbu The upper time base DWORD
 * @param tbl The lower time base DWORD
 */
void ppc_set_tbr(const uint32_t tbu, const uint32_t tbl);

/**
 * For a given clock speed and ticks for the TBR, calculate
 * the number of nanoseconds per timebase register tick
 * @param clock_hz The clock speed of the processor
 * @param ticks_per_clock The number of timebase register 'ticks' per clock cycle
 * @return The number of nanoseconds per timebase register 'tick'
 */
uint32_t ppc_get_ns_per_tb_tick(const uint64_t * const clock_hz, const uint32_t ticks_per_clock);

/**
 * Set the value of the decrementer
 */
void ppc_set_decrementer(const uint32_t v);

/**
 * The place where the IVT routines will end up
 * once they've saved the context
 */
void ppc_isr_handler(const uint32_t vector, tgt_context_t * const registers);

/**
 * Setup the powerpc instruction breakpoint register IDAR
 */
void ppc_set_instruction_breakpoint(const uint32_t location);

/**
 * Setup the powerpc data breakpoint register DDAR
 */
void ppc_set_data_breakpoint(const uint32_t location);

uint32_t ppc_get_dsisr(void);

#endif /* POWERPC32_H_ */
