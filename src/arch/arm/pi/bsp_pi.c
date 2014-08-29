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

#include "arch/arm/arm.h"

#include "kernel/kernel_initialise.h"
#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"

#include "uart.h"

static timer_t arm_internal_clock;

void bsp_initialise(void)
{
	uart_init();

	/*
	 * Initialise the Target Processor
	 */
	tgt_initialise();

	/*
	 * Initialise the Interrupt Vector Table
	 */
	ivt_initialise();

#if defined(KERNEL_DEBUGGING)
	uart_puts("UART 16550 Port 1 Up\n\0");
#endif

	// TODO Initialise the MMU
	// TODO Enable the MMU
}

void bsp_setup(void)
{
	//opic_intc = opic_intc_create(mem_get_default_pool(), (void*)0x80000000);
	//int_install_isr(opic_intc);

	alarm_set_timer(&arm_internal_clock);

	// route UART -> OPIC -> CPU
	//intc_enable(opic_intc, 1);
	//intc_add_device(opic_intc, 1, &rs232_port_1);

	// enable UART interrupts
	//rs232_port_1.write_register(UART_1_BASE_ADDRESS, 1, 1);
}

void bsp_enable_schedule_timer(void)
{
    // TODO schedule an alarm to generate an external
    // interrupt at the scheduling rate
}

void arm_check_timer(timer_t * const t)
{
	// TODO move this into generic arm code
}

void bsp_check_timers_and_alarms(void)
{
	arm_check_timer(&arm_internal_clock);
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
	//rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, (void*)&c, 1);
	uart_putc(c);
}

char bsp_read_debug_char(void)
{
	char c = 0;
	while (c == 0)
	{
		c = uart_getc();
	}
	return c;
}
