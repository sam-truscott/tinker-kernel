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
#include "arch/arm/arm_vec.h"
#include "arch/arm/arm_cpsr.h"

#include "kernel/kernel_initialise.h"
#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"
#include "kernel/utils/util_memset.h"
#include "devices/timer/bcm2835/bcm2835_timer.h"
#include "devices/serial/bcm2835/bcm2835_uart.h"
#include "devices/intc/bcm2835/bcm2835_intc.h"

static timer_t bcm2835_scheduler_timer;
static timer_t bcm2835_system_timer;
static tinker_time_t scheduler_time;
static tinker_time_t scheduler_period;
static intc_t * bcm2835_intc;
static kernel_device_t uart;

void bsp_initialise(void)
{
	bcm2835_uart_init();

#if defined(TARGET_DEBUGGING)
	bcm2835_uart_puts("UART Up\n\0");
#endif

	/* Initialise the Target Processor */
	tgt_initialise();

	/* Initialise the Interrupt Vector Table */
	ivt_initialise();

	// TODO Initialise the MMU
	// TODO Enable the MMU
}

void bsp_setup(void)
{
	bcm2835_intc = bcm2835_intc_create(mem_get_default_pool(), (void*)0x2000B000);
	int_install_isr(bcm2835_intc);

	time_set_system_clock(bcm2835_get_clock((void*)0x20003000, mem_get_default_pool()));

	bcm2835_get_timer(mem_get_default_pool(), &bcm2835_scheduler_timer, (void*)0x20003000, 1);

	bcm2835_get_timer(mem_get_default_pool(), &bcm2835_system_timer, (void*)0x20003000, 3);
	alarm_set_timer(&bcm2835_system_timer);

	intc_add_timer(bcm2835_intc, INTERRUPT_TIMER1, &bcm2835_scheduler_timer);
	intc_add_timer(bcm2835_intc, INTERRUPT_TIMER3, &bcm2835_system_timer);

	intc_enable(bcm2835_intc, INTERRUPT_TIMER1);
	intc_enable(bcm2835_intc, INTERRUPT_TIMER3);

	bcm2835_uart_get_device(&uart);
	intc_add_device(bcm2835_intc, INTERRUPT_UART, &uart);
	intc_enable(bcm2835_intc, INTERRUPT_VC_UART);

#if defined(TARGET_DEBUGGING)
	debug_print("BSP: Enabling external interrupts\n");
#endif
}

static void arm_vec_handler(arm_vec_t type, uint32_t contextp);

void ivt_initialise(void)
{
    // write values into the vector table
	arm_vec_install(VECTOR_RESET, &arm_vec_handler);
	arm_vec_install(VECTOR_UNDEFINED, &arm_vec_handler);
	arm_vec_install(VECTOR_SYSTEM_CALL, &arm_vec_handler);
	arm_vec_install(VECTOR_PRETECH_ABORT, &arm_vec_handler);
	arm_vec_install(VECTOR_DATA_ABORT, &arm_vec_handler);
	arm_vec_install(VECTOR_RESERVED, &arm_vec_handler);
	arm_vec_install(VECTOR_IRQ, &arm_vec_handler);
	arm_vec_install(VECTOR_FIQ, &arm_vec_handler);
}

static void arm_vec_handler(arm_vec_t type, uint32_t contextp)
{
#if defined(TARGET_DEBUGGING)
	debug_print("BSP: Vector %d\n", type);
#endif
	tgt_context_t * const context = (tgt_context_t*)contextp;
	switch(type)
	{
	case VECTOR_RESET:
	case VECTOR_UNDEFINED:
	case VECTOR_PRETECH_ABORT:
	case VECTOR_DATA_ABORT:
	case VECTOR_RESERVED:
		int_fatal_program_error_interrupt(context);
		break;
	case VECTOR_SYSTEM_CALL:
		int_syscall_request_interrupt(context);
		break;
	case VECTOR_IRQ:
	case VECTOR_FIQ:
	{
		const error_t handled = int_handle_external_vector(context);
		if (handled != NO_ERROR)
		{
			debug_print("BSP: Failed to handle external interrupt, error = %d\n", handled);
		}
	}
		break;
	default:
		debug_print("BSP: Unknown interrupt type %d\n", type);
		break;
	}
}

static void bsp_scheduler_timeout(tgt_context_t * const context)
{
#if defined(TARGET_DEBUGGING)
	debug_print("BSP: ----------------------\n");
	debug_print("BSP: Scheduler timeout\n");
	debug_print("BSP: Mode %d\n", arm_get_psr_mode());
	debug_print("BSP: %x %x %x %x %x\n", context->gpr[0], context->gpr[1], context->gpr[2], context->gpr[3], context->gpr[4]);
	debug_print("BSP: %x %x %x %x %x\n", context->gpr[5], context->gpr[6], context->gpr[7], context->gpr[8], context->gpr[9]);
	debug_print("BSP: %x %x %x\n", context->gpr[10], context->gpr[11], context->gpr[12]);
	debug_print("BSP: sp %x lr %x\n", context->sp, context->lr);
	debug_print("BSP: ----------------------\n");
#endif
	int_context_switch_interrupt(context);
#if defined(TARGET_DEBUGGING)
	debug_print("BSP: ----------------------\n");
	debug_print("BSP: Scheduler timeout done\n");
	debug_print("BSP: Mode %d\n", arm_get_psr_mode());
	debug_print("BSP: %x %x %x %x %x\n", context->gpr[0], context->gpr[1], context->gpr[2], context->gpr[3], context->gpr[4]);
	debug_print("BSP: %x %x %x %x %x\n", context->gpr[5], context->gpr[6], context->gpr[7], context->gpr[8], context->gpr[9]);
	debug_print("BSP: %x %x %x\n", context->gpr[10], context->gpr[11], context->gpr[12]);
	debug_print("BSP: sp %x lr %x\n", context->sp, context->lr);
	debug_print("BSP: ----------------------\n");
#endif
}

void bsp_enable_schedule_timer(void)
{
	tinker_time_t now;
	time_get_system_time(&now);
	tinker_time_milliseconds(100, &scheduler_period);
	tinker_time_add(&now, &scheduler_period, &scheduler_time);
	bcm2835_scheduler_timer.timer_setup(bcm2835_scheduler_timer.usr_data, &scheduler_time, &bsp_scheduler_timeout);
}

void bsp_check_timers_and_alarms(void)
{
	// unused - we don't need this as we have two timers
}

uint32_t bsp_get_usable_memory_start()
{
	extern uint32_t end;
	return (uint32_t)&end;
}

uint32_t bsp_get_usable_memory_end()
{
	/* 127 because the last 1MB is the page table */
	return (258 * 1024 * 1024);
}

void bsp_write_debug_char(const char c)
{
	//rs232_port_1.write_buffer(UART_1_BASE_ADDRESS,0, (void*)&c, 1);
	bcm2835_uart_putc(c);
}

char bsp_read_debug_char(void)
{
	char c = 0;
	while (c == 0)
	{
		c = bcm2835_uart_getc();
	}
	return c;
}

void tgt_wait_for_interrupt(void)
{
	asm("MOV r0, #0 ");
	asm("MCR p15, 0, r0, c7, c0, 4");
}
