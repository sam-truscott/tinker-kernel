/*
 *
 * TINKER Source Code
 * 
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
static kernel_device_t rs232_port_1;

static intc_t * opic_intc;

static timer_t arm_internal_clock;

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

	uart16550_get_device(UART_1_BASE_ADDRESS, &rs232_port_1);

#if defined(KERNEL_DEBUGGING)
	rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, "UART 16550 Port 1 Up\n\0", 21);
#endif

	// TODO Initialise the MMU
	// TODO Enable the MMU
}

void bsp_setup(void)
{
	rs232_port_1.initialise(&rs232_port_1, NULL, 0);

	opic_intc = opic_intc_create(mem_get_default_pool(), (void*)0x80000000);
	int_install_isr(opic_intc);

	alarm_set_timer(&arm_internal_clock);

	// route UART -> OPIC -> CPU
	intc_enable(opic_intc, 1);
	intc_add_device(opic_intc, 1, &rs232_port_1);

	// enable UART interrupts
	//rs232_port_1.write_register(UART_1_BASE_ADDRESS, 1, 1);
}

void bsp_enable_schedule_timer(void)
{
    // TODO schedule an alarm to generate an external
    // interrupt at the scheduling rate
}

void bsp_check_timers_and_alarms(void)
{
	ppc_check_timer(&arm_internal_clock);
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

char bsp_read_debug_char(void)
{
	char c = 0;
	while (c == 0)
	{
		rs232_port_1.read_buffer(UART_1_BASE_ADDRESS,0,(void*)&c,1);
	}
	return c;
}
