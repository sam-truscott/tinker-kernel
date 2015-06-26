/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arch/board_support.h"
#include "arch/ppc32/powerpc32.h"

#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_i_to_a.h"

static char bsp_vector_names[15][27] =
{
		"\0",
		"SYSTEM RESET\0",
		"MACHINE CHECK\0",
		"DATA STORAGE\0",
		"INSTRUCTION STORAGE\0",
		"EXTERNAL INTERRUPT\0",
		"ALIGNMENT\0",
		"PROGRAM ERROR\0",
		"FLOATING POINT UNAVAILABLE\0",
		"DECREMENTER\0",
		"SYSCALL\0",
		"TRACE\0",
		"FLOATING POINT ASSIST\0"
};

static void bsp_print_str(char * str);

static void bsp_print_hex(uint32_t i);

void bsp_print_str(char * str)
{
	const uint32_t len = util_strlen(str, 255);
	uint32_t p = 0;
	while(p<len)
	{
		bsp_write_debug_char(str[p]);
		p++;
	}
}

void bsp_print_hex(uint32_t i)
{
	char reg_str[11] = {'0','x',0,0,0,0,0,0,0,0,0};
	util_i_to_h(i, reg_str + 2, 8);
	bsp_print_str(reg_str);
}

void bsp_decrementer_interrupt(
		const uint32_t vector,
		tgt_context_t * const context)
{
	asm volatile("mfdec %r10");
	if (vector == ppc32_vector_decrementer)
	{
		/* switch the context afterwards as what we change to may
		 * have changed because of any alarms/timers */
		int_context_switch_interrupt(context);
	}
}

void bsp_fatal_program_error(
		const uint32_t vector,
		tgt_context_t * const context)
{
	if (vector)
	{
		char id[5] = {0,0,0,0,0};
		char * name = bsp_vector_names[vector];

		bsp_print_str("BSP FATAL EXCEPTION \0");

		util_i_to_a(vector, id, 4);
		bsp_print_str(id);
		bsp_print_str(" \0");
		bsp_print_str(name);
		bsp_print_str("\n\0");

		const uint32_t dsisr = ppc_get_dsisr();
		const uint32_t srr0 = ppc_get_srr0();
		const uint32_t srr1 = ppc_get_srr1();

		bsp_print_str("\n\0");

		bsp_print_str("SRR0 = ");
		bsp_print_hex(srr0);
		bsp_print_str("\n\0");

		bsp_print_str("SRR1 = ");
		bsp_print_hex(srr1);
		bsp_print_str("\n\0");

		bsp_print_str("DSISR = ");
		bsp_print_hex(dsisr);
		bsp_print_str("\n\n\0");

		int_fatal_program_error_interrupt(context);
	}
}

void bsp_system_call_request(
		const uint32_t vector,
		tgt_context_t * const context)
{
	// ensure external interrupts are disabled but make it recoverable
	// this will ensure that the any screw ups in the syscall generates
	// a proper exception and shuts down that thread properly.
	ppc_set_msr((ppc_get_msr() | MSR_FLAG_RI) & ~MSR_FLAG_EE);
	if ( vector == ppc32_vector_syscall )
	{
		int_syscall_request_interrupt(context);
	}
}
