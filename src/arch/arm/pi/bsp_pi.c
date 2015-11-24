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
#include "arch/arm/arm_mmu.h"

#include "kernel/kernel_initialise.h"
#include "kernel/kernel_assert.h"
#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_list.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"
#include "kernel/utils/util_memset.h"
#include "devices/timer/bcm2835/bcm2835_timer.h"
#include "devices/serial/bcm2835/bcm2835_uart.h"
#include "devices/intc/bcm2835/bcm2835_intc.h"
#include "kernel/shell/kshell.h"

#define UART_DEVICE_NAME "/dev/char/bcm2835_uart"

static timer_t bcm2835_scheduler_timer;
static timer_t bcm2835_system_timer;
static tinker_time_t scheduler_time;
static tinker_time_t scheduler_period;
static intc_t * bcm2835_intc = NULL;
static kernel_device_t uart;
static interrupt_controller_t * interrupt_controller = NULL;
static time_manager_t * time_manager = NULL;

void bsp_initialise(void)
{
	early_uart_init();
#if defined (TARGET_DEBUGGING)
	early_uart_put("BSP\n");
	printp_out("CPSR %x\n", arm_get_cpsr());
#endif

	/* Initialise the Target Processor */
	tgt_initialise();

#if defined (TARGET_DEBUGGING)
	early_uart_put("Target init done\n");
#endif

	/* Initialise the Interrupt Vector Table */
	ivt_initialise();

#if defined (TARGET_DEBUGGING)
	early_uart_put("Vector init done\n");
#endif
}

void bsp_setup(
		interrupt_controller_t * const controller,
		time_manager_t * const tm,
		alarm_manager_t * const am)
{
	interrupt_controller = controller;
	time_manager = tm;

	arm_invalidate_all_tlbs();
	arm_set_translation_table_base(process_get_page_table(kernel_get_process()));
	arm_enable_mmu();

	bcm2835_uart_get_device(&uart, UART_DEVICE_NAME);
	kshell_set_input_device(UART_DEVICE_NAME);

	bcm2835_intc = bcm2835_intc_create(mem_get_default_pool(), (void*)0x2000B000);
	int_install_isr(controller, bcm2835_intc);

	time_set_system_clock(tm, bcm2835_get_clock((void*)0x20003000, mem_get_default_pool()));

	bcm2835_get_timer(mem_get_default_pool(), &bcm2835_scheduler_timer, (void*)0x20003000, 1);
	bcm2835_get_timer(mem_get_default_pool(), &bcm2835_system_timer, (void*)0x20003000, 3);

	alarm_set_timer(am, &bcm2835_system_timer);

	intc_add_timer(bcm2835_intc, INTERRUPT_TIMER1, &bcm2835_scheduler_timer);
	intc_add_timer(bcm2835_intc, INTERRUPT_TIMER3, &bcm2835_system_timer);
	intc_add_device(bcm2835_intc, INTERRUPT_UART, &uart);

#if defined(TARGET_DEBUGGING)
	arm_print_page_table(process_get_page_table(kernel_get_process()));
#endif

	intc_enable(bcm2835_intc, INTERRUPT_TIMER1);
	intc_enable(bcm2835_intc, INTERRUPT_TIMER3);
	intc_enable(bcm2835_intc, INTERRUPT_VC_UART);
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
	kernel_assert("interrupt controller not set in bsp\n", interrupt_controller != NULL);
	tgt_context_t * const context = (tgt_context_t*)contextp;
	switch(type)
	{
	case VECTOR_RESET:
	case VECTOR_UNDEFINED:
	case VECTOR_PRETECH_ABORT:
	case VECTOR_DATA_ABORT:
	case VECTOR_RESERVED:
		int_fatal_program_error_interrupt(interrupt_controller, context);
		break;
	case VECTOR_SYSTEM_CALL:
		int_syscall_request_interrupt(interrupt_controller, context);
		break;
	case VECTOR_IRQ:
	case VECTOR_FIQ:
	{
		const error_t handled = int_handle_external_vector(interrupt_controller, context);
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

static void bsp_scheduler_timeout(tgt_context_t * const context, void * const param)
{
	(void)param; // UNUSED
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
	int_context_switch_interrupt(interrupt_controller, context);
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
	time_get_system_time(time_manager, &now);
	tinker_time_milliseconds((1000 / SCHEDULER_HERTZ), &scheduler_period);
	tinker_time_add(&now, &scheduler_period, &scheduler_time);
	bcm2835_scheduler_timer.timer_setup(
			bcm2835_scheduler_timer.usr_data,
			&scheduler_time,
			&bsp_scheduler_timeout,
			NULL);
}

uint32_t bsp_get_usable_memory_start()
{
	extern uint32_t end;
	return (uint32_t)&end;
}

uint32_t bsp_get_usable_memory_end()
{
	return (128 * 1024 * 1024);
}

void bsp_write_debug_char(const char c)
{
	if (uart.user_data)
	{
		uart.write_register(uart.user_data, 0, c);
	}
	else
	{
		early_uart_putc(c);
	}
}

void tgt_wait_for_interrupt(void)
{
	asm volatile("MOV r0, #0");
	asm volatile("MCR p15, 0, r0, c7, c0, 4");
}
