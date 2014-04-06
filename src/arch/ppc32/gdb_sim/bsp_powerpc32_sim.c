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
#include "arch/ppc32/generic/ppc32_generic_bsp.h"

#include "kernel/kernel_initialise.h"
#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"

#include "devices/serial/uart16550/uart16550.h"
#include "devices/intc/opic/opic_intc.h"
#include "devices/timer/ppc32tbr/ppc32tbr_timer.h"

#define UART_1_BASE_ADDRESS (void*)(0xf40002F8)
#define UART_2_BASE_ADDRESS (void*)(0xf40003F8)

/**
 * The device information for port 1 of the UART 16550
 */
static __kernel_device_t __rs232_port_1;

static __intc_t * __opic_intc;

static __timer_t __ppc32_time_base_timer;


/**
 * External Interrupt by a Hardware Device
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
static void __bsp_external_interrupt(
		const uint32_t vector,
		__tgt_context_t * const context);

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

	__uart16550_get_device(UART_1_BASE_ADDRESS, &__rs232_port_1);

#if defined(__KERNEL_DEBUGGING)
	__rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, "UART 16550 Port 1 Up\n\0", 21);
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

	/* setup the SDR1 for the page table - based on
	 * Minimum Recommended Page Table Sizes from PowerPC
	 * Programming Environments Manual  */
	/*
	__ppc32_set_sdr1(__PPC_SDR(__page_table, 0xF));

	extern char * __utext;
	extern char * __udata;
	char * user_text_pos = (char*)&__utext;
	char * user_data_pos = (char*)&__udata;

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
					1,
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
					1,
					0,
					__PPC32_WIMG(
							__ppc32_write_back,
							__ppc32_cache_enabled,
							__ppc32_memory_no_coherency,
							__ppc32_not_guarded),
					__ppc32_read_write));
	*/

	{
		uint32_t msr = __ppc_get_msr();
		msr |= (MSR_FLAG_IR | MSR_FLAG_DR);
		__ppc_set_msr(msr);
	}
}

void __bsp_setup(void)
{
	__rs232_port_1.initialise(&__rs232_port_1, NULL, 0);

	__opic_intc = __opic_intc_create(__mem_get_default_pool(), (void*)0x80000000);
	__int_install_isr(__opic_intc);

	/* 1Ghz clock with 64 time-base ticks per clock.
	 * This doesn't match the PSIM simulation but it doesn't
	 * matter */
	const uint64_t sim_clock = 1 * 1000 * 1000 * 1000;
	__ppc_setup_timebase_details(&sim_clock, 64);
	__time_set_system_clock(__ppc_get_ppc_timebase_clock());

	__ppc_get_timer(__kernel_get_process(), &__ppc32_time_base_timer);
	__alarm_set_timer(&__ppc32_time_base_timer);

	// route UART -> OPIC -> CPU
	__intc_enable(__opic_intc, 1);
	__intc_add_device(__opic_intc, 1, &__rs232_port_1);

	// enable UART interrupts
	//__rs232_port_1.write_register(UART_1_BASE_ADDRESS, 1, 1);
}

void __bsp_enable_schedule_timer(void)
{
	sos_time_t scheduler_time;
	sos_time_milliseconds(1000 / SCHEDULER_HERTZ, &scheduler_time);
	uint64_t tbr_value = 0;
	__ppc_convert_time_to_tbr(&scheduler_time, &tbr_value);
	__ppc_set_decrementer((uint32_t)tbr_value);
}

static void __bsp_external_interrupt(
		const uint32_t vector,
		__tgt_context_t * const context)
{
	if (vector && context) {}
	__int_handle_external_vector();
}

void __bsp_check_timers_and_alarms(void)
{
	__ppc_check_timer(&__ppc32_time_base_timer);
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
	__rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, (void*)&c, 1);
}

char __bsp_read_debug_char(void)
{
	char c = 0;
	while (c == 0)
	{
		__rs232_port_1.read_buffer(UART_1_BASE_ADDRESS,0,(void*)&c,1);
	}
	return c;
}
