/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "arch/ivt.h"
#include "arch/tgt.h"

#include "arch/ppc32/powerpc32.h"
#include "arch/ppc32/powerpc_clock.h"
#include "arch/ppc32/powerpc32_7400.h"
#include "arch/ppc32/powerpc32_mmu.h"

#include "kernel/kernel_initialise.h"
#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"
#include "kernel/utils/util_strlen.h"
#include "kernel/utils/util_i_to_h.h"

#include "devices/serial/uart16550/uart16550.h"
#include "devices/interrupt_controller/opic/opic_intc.h"
#include "devices/timer/ppc32tbr/ppc32tbr_timer.h"

#define UART_1_BASE_ADDRESS (void*)(0xf40002F8)
#define UART_2_BASE_ADDRESS (void*)(0xf40003F8)

static void __bsp_print_str(char * str);

static void __bsp_print_hex(uint32_t i);

/**
 * External Interrupt by a Hardware Device
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
static void __bsp_external_interrupt(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

/**
 * Decrementer Interrupt by the PowerPC
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
static void __bsp_decrementer_interrupt(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

/**
 * All program error interrupts
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
static void __bsp_fatal_program_error(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

/**
 * Requests from programs to perform system operations
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
static void __bsp_system_call_request(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled);

/**
 * The device information for port 1 of the UART 16550
 */
static __kernel_device_t rs232_port_1;

static __kernel_device_t opic_intc;

static __timer_t ppc32_time_base_timer;

static __ppc32_pteg_t ptegs[16 * 1024] __attribute__((section(".page_table"))); /* 16k of entries */

static __ppc32_pt_t page_table;

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

void __bsp_initialise(void)
{
	/*
	 * Initialise the Target Processor
	 */
	__tgt_initialise();

	/*
	 * Initialise the Interrupt Vector Table
	 */
	__ivt_initialise();

	/*
	 * Setup where all the interrupts go to
	 */
	__ppc_isr_attach(__ppc32_vector_system_reset, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_machine_check, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_data_storage, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_instruction_storage, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_external_interrupt, &__bsp_external_interrupt);
	__ppc_isr_attach(__ppc32_vector_alignment, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_program_error, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_floating_point_unavailable, &__bsp_fatal_program_error);
	__ppc_isr_attach(__ppc32_vector_decrementer, &__bsp_decrementer_interrupt);
	__ppc_isr_attach(__ppc32_vector_syscall, &__bsp_system_call_request);
	/* Not supported __ppc_isr_attach(__ppc32_vector_trace); */
	/* Not supported __ppc_isr_attach(__ppc32_vector_floating_point_assist); */

	uart16550_get_device(UART_1_BASE_ADDRESS, &rs232_port_1);

#if defined(__KERNEL_DEBUGGING)
	rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, "UART 16550 Port 1 Up\n\0", 21);
#endif

	/* IBAT0 Setup for RAM */
	__ppc32_set_ibat0l(
			__PPC32_BATL(
					0,
					__PPC32_WIMG(	__ppc32_write_back,
									__ppc32_cache_enabled,
									__ppc32_memory_no_coherency,
									__ppc32_not_guarded),
					__ppc32_read_write));

	__ppc32_set_ibat0u(
			__PPC32_BATU(
					0,
					__ppc32_mb_128,
					__ppc32_supervisor_access,
					__ppc32_no_user_access));

	/* DBAT0 Setup for RAM */
	__ppc32_set_dbat0l(
			__PPC32_BATL(
					0,
					__PPC32_WIMG(	__ppc32_write_back,
									__ppc32_cache_enabled,
									__ppc32_memory_no_coherency,
									__ppc32_not_guarded),
					__ppc32_read_write));

	__ppc32_set_dbat0u(
			__PPC32_BATU(
					0,
					__ppc32_mb_128,
					__ppc32_supervisor_access,
					__ppc32_no_user_access));

	/* DBAT1 Setup for Interrupt Controller */
	__ppc32_set_dbat1l(
			__PPC32_BATL(
					__PPC32_BEPI_BRPN(0x80000000u),
					__PPC32_WIMG(	__ppc32_write_through,
									__ppc32_cache_inhibited,
									__ppc32_memory_no_coherency,
									__ppc32_guarded),
					__ppc32_read_write));

	__ppc32_set_dbat1u(
			__PPC32_BATU(
					__PPC32_BEPI_BRPN(0x80000000u),
					__ppc32_kb_128,
					__ppc32_supervisor_access,
					__ppc32_no_user_access));

	/* DBAT2 Setup for Serial */
	__ppc32_set_dbat1l(
			__PPC32_BATL(
					__PPC32_BEPI_BRPN(0xf40002F8u),
					__PPC32_WIMG(	__ppc32_write_through,
									__ppc32_cache_inhibited,
									__ppc32_memory_no_coherency,
									__ppc32_guarded),
					__ppc32_read_write));

	__ppc32_set_dbat1u(
			__PPC32_BATU(
					__PPC32_BEPI_BRPN(0xf40002F8u),
					__ppc32_kb_128,
					__ppc32_supervisor_access,
					__ppc32_no_user_access));

	/* setup all the SR registers - only first segment is setup for pages */
	__ppc32_set_sr0(__PPC_SR_T0(SR_KS_OK, SR_KP_OK, SR_NE_OFF, 0));
	__ppc32_set_sr1(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr2(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr3(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr4(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr5(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr6(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr7(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr8(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr9(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr10(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr11(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr12(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr13(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr14(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	__ppc32_set_sr15(__PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));

	page_table = ptegs; /* pointer to page table */;

	/* setup the SDR1 for the page table - based on
	 * Minimum Recommended Page Table Sizes from PowerPC
	 * Programming Environments Manual  */
	__ppc32_set_sdr1(__PPC_SDR(page_table, 0xF));

	extern char * __user_text;
	extern char * __user_data;
	char * user_text_pos = (char*)&__user_text;
	char * user_data_pos = (char*)&__user_data;

	__ppc32_add_pte(
			(uint32_t)user_text_pos,
			16,
			__PPC_PTE_W0(
					PTE_VALID,
					16,
					HASH_PRIMARY,
					0),
			__PPC_PTE_W1(
					(uint32_t)user_text_pos,
					0,
					0,
					__PPC32_WIMG(
							__ppc32_write_back,
							__ppc32_cache_enabled,
							__ppc32_memory_no_coherency,
							__ppc32_not_guarded),
					__ppc32_read_write));

	__ppc32_add_pte(
			(uint32_t)user_data_pos,
			16,
			__PPC_PTE_W0(
					PTE_VALID,
					16,
					HASH_PRIMARY,
					0),
			__PPC_PTE_W1(
					(uint32_t)user_data_pos,
					0,
					0,
					__PPC32_WIMG(
							__ppc32_write_back,
							__ppc32_cache_enabled,
							__ppc32_memory_no_coherency,
							__ppc32_not_guarded),
					__ppc32_read_write));

	{
		uint32_t msr = __ppc_get_msr();
		msr |= (MSR_FLAG_IR | MSR_FLAG_DR);
		__ppc_set_msr(msr);
	}
}

void __bsp_setup(void)
{
	rs232_port_1.initialise(&rs232_port_1, NULL, 0);

	opic_intc_get_device((void*)0x80000000, &opic_intc);

	/* 1Ghz clock with 64 time-base ticks per clock.
	 * This doesn't match the PSIM simulation but it doesn't
	 * matter */
	__ppc_setup_timebase_details(1 * 1000 * 1000 * 1000, 64);
	__time_set_system_clock( __ppc_get_ppc_timebase_clock() );

	__ppc_get_timer(__kernel_get_process(), &ppc32_time_base_timer);
	__alarm_set_timer(&ppc32_time_base_timer);
}

void __bsp_enable_schedule_timer(void)
{
	/** TODO this should be build from a constant
	 * calculated form config.h */
	/* __ppc_set_decrementer(10000); */
	__ppc_set_decrementer(100000);
}

uint32_t __bsp_get_usable_memory_start()
{
	extern uint32_t end;
	return (uint32_t)&end;
}

uint32_t __bsp_get_usable_memory_end()
{
	/* 127 because the last 1MB is the page table */
	return (127 * 1024 * 1024);
}

void __bsp_write_debug_char(const char c)
{
	rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, (void*)&c, 1);
}

char __bsp_read_debug_char(void)
{
	char c = 0;
	while (c == 0 )
	{
		rs232_port_1.read_buffer(UART_1_BASE_ADDRESS,0,(void*)&c,1);
	}
	return c;
}

/*
 * Internal stuff
 */

static void __bsp_external_interrupt(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	if ( vector && context && fp_enabled ) {}
	const uint32_t external_vector = opic_ack(&opic_intc);
	__int_handle_external_vector(external_vector);
	opic_end_isr(&opic_intc, external_vector);
}

static void __bsp_decrementer_interrupt(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	__asm__("mfdec %r10");
	if ( vector == __ppc32_vector_decrementer )
	{
		/* check for any timers that may have expired */
		__ppc_check_timer(&ppc32_time_base_timer);

		/* FIXME context_size should depend on bool fp_enabled */
		if ( fp_enabled ) {}

		/* switch the context afterwards as what we change to may
		 * have changed because of any alarms/timers */
		__int_context_switch_interrupt(context);
	}
}

static void __bsp_fatal_program_error(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	if ( vector && fp_enabled )
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

		switch(vector)
		{
			case __ppc32_vector_data_storage:
				{
					uint32_t dsisr;
					uint32_t srr0;
					uint32_t srr1;

					__bsp_print_str("\n\0");

					dsisr = __ppc_get_dsisr();
					srr0 = __ppc_get_srr0();
					srr1 = __ppc_get_srr1();

					__bsp_print_str("SRR0 = ");
					__bsp_print_hex(srr0);
					__bsp_print_str("\n\0");

					__bsp_print_str("SRR1 = ");
					__bsp_print_hex(srr1);
					__bsp_print_str("\n\0");

					__bsp_print_str("DSISR = ");
					__bsp_print_hex(dsisr);
					__bsp_print_str("\n\n\0");
				}
				break;
		}

		__int_fatal_program_error_interrupt(context);
	}
}

static void __bsp_system_call_request(
		uint32_t vector,
		__tgt_context_t * context,
		bool_t fp_enabled)
{
	if ( fp_enabled ) {}
	if ( vector == __ppc32_vector_syscall )
	{
		__int_syscall_request_interrupt(context);
	}
}

static void __bsp_print_str(char * str)
{
	uint32_t len = __util_strlen(str, 255);
	rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, str, len);
}

static void __bsp_print_hex(uint32_t i)
{
	char reg_str[11] = {'0','x',0,0,0,0,0,0,0,0,0};
	__util_i_to_h(i, reg_str + 2, 8);
	__bsp_print_str(reg_str);
}
