/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arch/board_support.h"
#include "arch/ppc32/powerpc32.h"

#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_i_to_h.h"

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

static void __bsp_print_str(char * str);

static void __bsp_print_hex(uint32_t i);

void __bsp_print_str(char * str)
{
	const uint32_t len = __util_strlen(str, 255);
	uint32_t p = 0;
	while(p<len)
	{
		__bsp_write_debug_char(str[p]);
		p++;
	}
}

void __bsp_print_hex(uint32_t i)
{
	char reg_str[11] = {'0','x',0,0,0,0,0,0,0,0,0};
	__util_i_to_h(i, reg_str + 2, 8);
	__bsp_print_str(reg_str);
}

void __bsp_decrementer_interrupt(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	__asm__("mfdec %r10");
	if ( vector == __ppc32_vector_decrementer )
	{
		/* check for any timers that may have expired */
		__bsp_check_timers_and_alarms();

		/* FIXME context_size should depend on bool fp_enabled */
		if ( fp_enabled ) {}

		/* switch the context afterwards as what we change to may
		 * have changed because of any alarms/timers */
		__int_context_switch_interrupt(context);
	}
}

void __bsp_fatal_program_error(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	if (fp_enabled) {}
	if (vector)
	{
		char id[2] = {0,0};
		char * name = bsp_vector_names[vector];

		__bsp_print_str("BSP FATAL EXCEPTION \0");

		if ( vector < 10 )
		{
			id[0] = vector + '0';
		}
		else
		{
			/* TODO Support debugging of fatal interrupts >= 10 vector ID */
		}
		__bsp_print_str(id);
		__bsp_print_str(" \0");
		__bsp_print_str(name);
		__bsp_print_str("\n\0");

		const uint32_t dsisr = __ppc_get_dsisr();
		const uint32_t srr0 = __ppc_get_srr0();
		const uint32_t srr1 = __ppc_get_srr1();

		__bsp_print_str("\n\0");

		__bsp_print_str("SRR0 = ");
		__bsp_print_hex(srr0);
		__bsp_print_str("\n\0");

		__bsp_print_str("SRR1 = ");
		__bsp_print_hex(srr1);
		__bsp_print_str("\n\0");

		__bsp_print_str("DSISR = ");
		__bsp_print_hex(dsisr);
		__bsp_print_str("\n\n\0");

		__int_fatal_program_error_interrupt(context);
	}
}

void __bsp_system_call_request(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	// ensure external interrupts are disabled but make it recoverable
	// this will ensure that the any screw ups in the syscall generates
	// a proper exception and shuts down that thread properly.
	__ppc_set_msr((__ppc_get_msr() | MSR_FLAG_RI) & ~MSR_FLAG_EE);
	if ( fp_enabled ) {}
	if ( vector == __ppc32_vector_syscall )
	{
		__int_syscall_request_interrupt(context);
	}
}
