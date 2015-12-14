/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "ivt.h"
#include "tgt.h"

#include "powerpc32.h"
#include "powerpc_clock.h"
#include "powerpc32_7400.h"
#include "powerpc32_mmu.h"
#include "ppc32_generic_bsp.h"

#include "kernel_initialise.h"
#include "interrupts/interrupt_manager.h"
#include "scheduler/scheduler.h"
#include "process/process_list.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"

#include "uart16550.h"
#include "opic_intc.h"

#define UART_1_BASE_ADDRESS (void*)(0xf40002F8)
#define UART_2_BASE_ADDRESS (void*)(0xf40003F8)

/**
 * The device information for port 1 of the UART 16550
 */
static kernel_device_t rs232_port_1;

static intc_t * opic_intc;

static timer_t opic_timer;


/**
 * External Interrupt by a Hardware Device
 * @param vector The interrupt vector from the controller
 * @param context The saved context from the interruption
 */
static void bsp_external_interrupt(
		const uint32_t vector,
		tgt_context_t * const context);

void bsp_initialise(void)
{
	/*
	 * Initialise the Target Processor
	 */
	tgt_initialise();

	/*
	 * Initialise the Interrupt Vector Table
	 */
	ivt_initialise();

	/*
	 * Setup where all the interrupts go to
	 */
	ppc_isr_attach(ppc32_vector_system_reset, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_machine_check, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_data_storage, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_instruction_storage, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_external_interrupt, &bsp_external_interrupt);
	ppc_isr_attach(ppc32_vector_alignment, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_program_error, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_floating_point_unavailable, &bsp_fatal_program_error);
	ppc_isr_attach(ppc32_vector_decrementer, &bsp_decrementer_interrupt);
	ppc_isr_attach(ppc32_vector_syscall, &bsp_system_call_request);
	/* Not supported ppc_isr_attach(ppc32_vector_trace); */
	/* Not supported ppc_isr_attach(ppc32_vector_floating_point_assist); */

	uart16550_get_device(UART_1_BASE_ADDRESS, &rs232_port_1);

#if defined(TARGET_DEBUGGING)
	rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, "UART 16550 Port 1 Up\n\0", 21);
#endif

	/* IBAT0 Setup for RAM */
	ppc32_set_ibat0l(
			PPC32_BATL(
					0,
					PPC32_WIMG(	ppc32_write_back,
									ppc32_cache_enabled,
									ppc32_memory_no_coherency,
									ppc32_not_guarded),
					ppc32_read_write));

	ppc32_set_ibat0u(
			PPC32_BATU(
					0,
					ppc32_mb_128,
					ppc32_supervisor_access,
					ppc32_no_user_access));

	/* DBAT0 Setup for RAM */
	ppc32_set_dbat0l(
			PPC32_BATL(
					0,
					PPC32_WIMG(	ppc32_write_back,
									ppc32_cache_enabled,
									ppc32_memory_no_coherency,
									ppc32_not_guarded),
					ppc32_read_write));

	ppc32_set_dbat0u(
			PPC32_BATU(
					0,
					ppc32_mb_128,
					ppc32_supervisor_access,
					ppc32_no_user_access));

	/* DBAT1 Setup for Interrupt Controller */
	ppc32_set_dbat1l(
			PPC32_BATL(
					PPC32_BEPI_BRPN(0x80000000u),
					PPC32_WIMG(	ppc32_write_through,
									ppc32_cache_inhibited,
									ppc32_memory_no_coherency,
									ppc32_guarded),
					ppc32_read_write));

	ppc32_set_dbat1u(
			PPC32_BATU(
					PPC32_BEPI_BRPN(0x80000000u),
					ppc32_kb_128,
					ppc32_supervisor_access,
					ppc32_no_user_access));

	/* DBAT2 Setup for Serial */
	ppc32_set_dbat1l(
			PPC32_BATL(
					PPC32_BEPI_BRPN(0xf40002F8u),
					PPC32_WIMG(	ppc32_write_through,
									ppc32_cache_inhibited,
									ppc32_memory_no_coherency,
									ppc32_guarded),
					ppc32_read_write));

	ppc32_set_dbat1u(
			PPC32_BATU(
					PPC32_BEPI_BRPN(0xf40002F8u),
					ppc32_kb_128,
					ppc32_supervisor_access,
					ppc32_no_user_access));

	/* setup all the SR registers - only first segment is setup for pages */
	ppc32_set_sr0(PPC_SR_T0(SR_KS_OK, SR_KP_OK, SR_NE_OFF, 0));
	ppc32_set_sr1(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr2(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr3(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr4(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr5(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr6(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr7(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr8(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr9(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr10(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr11(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr12(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr13(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr14(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));
	ppc32_set_sr15(PPC_SR_T0(SR_KS_FAIL, SR_KP_FAIL, SR_NE_OFF, 0));

	{
		uint32_t msr = ppc_get_msr();
		msr |= (MSR_FLAG_IR | MSR_FLAG_DR);
		ppc_set_msr(msr);
	}
}

void bsp_setup(void)
{
	rs232_port_1.initialise(&rs232_port_1, NULL, 0);

	opic_intc = opic_intc_create(mem_get_default_pool(), (void*)0x80000000);
	int_install_isr(opic_intc);

	/* 1Ghz clock with 64 time-base ticks per clock.
	 * This doesn't match the PSIM simulation but it doesn't
	 * matter */
	const uint64_t sim_clock = 1 * 1000 * 1000 * 1000;
	ppc_setup_timebase_details(&sim_clock, 64);
	time_set_system_clock(ppc_get_ppc_timebase_clock());

	opic_tmr_get_timer(kernel_get_process(), &opic_timer);
	alarm_set_timer(&opic_timer);

	// route UART -> OPIC -> CPU
	intc_setup(opic_intc, 1, 1, INTC_LEVEL, INTC_ACTIVE_HIGH);
	intc_enable(opic_intc, 1);
	intc_add_device(opic_intc, 1, &rs232_port_1);

	// enable UART interrupts
	//rs232_port_1.write_register(UART_1_BASE_ADDRESS, 1, 1);
}

void bsp_enable_schedule_timer(void)
{
    tinker_time_t scheduler_time;
    tinker_time_milliseconds(1000 / SCHEDULER_HERTZ, &scheduler_time);
	uint64_t tbr_value = 0;
	ppc_convert_time_to_tbr(&scheduler_time, &tbr_value);
	ppc_set_decrementer((uint32_t)tbr_value);
}

static void bsp_external_interrupt(
		const uint32_t vector,
		tgt_context_t * const context)
{
	(void)vector;
	int_handle_external_vector(context);
}

uint32_t bsp_get_usable_memory_start()
{
	extern uint32_t end;
	return (uint32_t)&end;
}

uint32_t bsp_get_usable_memory_end()
{
	/* 127 because the last 1MB is the page table */
	return (127 * 1024 * 1024);
}

void bsp_write_debug_char(const char c)
{
	rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, (void*)&c, 1);
}
