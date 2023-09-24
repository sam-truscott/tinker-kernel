/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "board_support.h"
#include "ivt.h"
#include "tgt.h"


#include "kernel_initialise.h"
#include "kernel_assert.h"
#include "interrupts/interrupt_manager.h"
#include "scheduler/scheduler.h"
#include "process/process_list.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"
#include "utils/util_memset.h"
#include "shell/kshell.h"

#include "console/print_out.h"

#include "memory/mem_section_private.h"
#include "memory/mem_section.h"

#define UART_DEVICE_NAME "/dev/char/bcm2835_uart"

static tinker_time_t scheduler_time;
static tinker_time_t scheduler_period;
static intc_t * bcm2835_intc = NULL;
static kernel_device_t uart;
static interrupt_controller_t * interrupt_controller = NULL;
static time_manager_t * time_manager = NULL;

void bsp_initialise(void)
{
	tgt_disable_external_interrupts();
	early_uart_init();
	if (is_debug_enabled(TARGET))
	{
		early_uart_put("BSP\n");
	}

	/* Initialise the Target Processor */
	tgt_initialise();

	if (is_debug_enabled(TARGET))
	{
		early_uart_put("Target init done\n");
	}

	/* Initialise the Interrupt Vector Table */
	ivt_initialise();

	if (is_debug_enabled(TARGET))
	{
		early_uart_put("Vector init done\n");
		debug_print(TARGET, "SP %8x, FP %8x\n", tgt_get_stack_pointer(), tgt_get_frame_pointer());
	}
}

void bsp_setup(
		interrupt_controller_t * const controller,
		time_manager_t * const tm,
		alarm_manager_t * const am,
		process_t * const kernel_process)
{
	interrupt_controller = controller;
	time_manager = tm;

	// init uart before setting the page table
#if defined(KERNEL_SHELL)
	// FIXME use object
	kshell_set_input_device(UART_DEVICE_NAME);
#endif

	// int_install_isr(controller, bcm2835_intc);
	// time_set_system_clock(tm, bcm2835_get_clock((void*)0x20003000, mem_get_default_pool()));
	// alarm_set_timer(am, &bcm2835_system_timer);
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
	debug_print(TARGET, "BSP: Vector %d\n", type);
	kernel_assert("interrupt controller not set in bsp\n", interrupt_controller != NULL);
	tgt_context_t * const context = (tgt_context_t*)contextp;

	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "BSP: ----------------------\n");
		debug_prints(TARGET, "BSP: VECTOR START\n");
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[0], context->gpr[1], context->gpr[2], context->gpr[3], context->gpr[4]);
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[5], context->gpr[6], context->gpr[7], context->gpr[8], context->gpr[9]);
		debug_print(TARGET, "BSP: %x %x %x\n", context->gpr[10], context->gpr[11], context->gpr[12]);
		debug_print(TARGET, "BSP: sp %x lr %x pc %x cpsr %x\n", context->sp, context->lr, context->pc, context->cpsr);
		debug_prints(TARGET, "BSP: ----------------------\n");
	}

	switch(type)
	{
	case VECTOR_DATA_ABORT:
		error_print("## ------------------S------------------------- ##\n");
		error_print("Data Abort sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		error_print("## ------------------E------------------------- ##\n");
		break;
	case VECTOR_RESET:
		error_print("## ------------------S------------------------- ##\n");
		error_print("Reset sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		error_print("## ------------------E------------------------- ##\n");
		break;
	case VECTOR_UNDEFINED:
		error_print("## ------------------S------------------------- ##\n");
		error_print("Undefined sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		error_print("## ------------------E------------------------- ##\n");
		break;
	case VECTOR_PRETECH_ABORT:
		error_print("## ------------------S------------------------- ##\n");
		error_print("Prefetch sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		error_print("## ------------------E------------------------- ##\n");
		break;
	case VECTOR_RESERVED:
		error_print("## ------------------S------------------------- ##\n");
		error_print("Reserved sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		error_print("## ------------------E------------------------- ##\n");
		break;
	case VECTOR_SYSTEM_CALL:
		int_syscall_request_interrupt(interrupt_controller, context);
		break;
	case VECTOR_IRQ:
	case VECTOR_FIQ:
		{
			const return_t handled = int_handle_external_vector(interrupt_controller, context);
			if (handled != NO_ERROR)
			{
				debug_print(TARGET, "BSP: Failed to handle external interrupt, error = %d\n", handled);
			}
		}
		break;
	default:
		debug_print(TARGET, "BSP: Unknown interrupt type %d\n", type);
		break;
	}

	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "BSP: ----------------------\n");
		debug_prints(TARGET, "BSP: VECTOR END\n");
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[0], context->gpr[1], context->gpr[2], context->gpr[3], context->gpr[4]);
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[5], context->gpr[6], context->gpr[7], context->gpr[8], context->gpr[9]);
		debug_print(TARGET, "BSP: %x %x %x\n", context->gpr[10], context->gpr[11], context->gpr[12]);
		debug_print(TARGET, "BSP: sp %x lr %x pc %x cpsr %x\n", context->sp, context->lr, context->pc, context->cpsr);
		debug_prints(TARGET, "BSP: ----------------------\n");
	}
}

static void bsp_scheduler_timeout(tgt_context_t * const context, void * const param)
{
	(void)param; // UNUSED
	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "BSP: ----------------------\n");
		debug_prints(TARGET, "BSP: Scheduler timeout\n");
		debug_print(TARGET, "BSP: Mode %d\n", arm_get_psr_mode());
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[0], context->gpr[1], context->gpr[2], context->gpr[3], context->gpr[4]);
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[5], context->gpr[6], context->gpr[7], context->gpr[8], context->gpr[9]);
		debug_print(TARGET, "BSP: %x %x %x\n", context->gpr[10], context->gpr[11], context->gpr[12]);
		debug_print(TARGET, "BSP: sp %x lr %x pc %x cpsr %x\n", context->sp, context->lr, context->pc, context->cpsr);
		debug_prints(TARGET, "BSP: ----------------------\n");
	}
	int_context_switch_interrupt(interrupt_controller, context);
	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "BSP: ----------------------\n");
		debug_prints(TARGET, "BSP: Scheduler timeout done\n");
		debug_print(TARGET, "BSP: Mode %d\n", arm_get_psr_mode());
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[0], context->gpr[1], context->gpr[2], context->gpr[3], context->gpr[4]);
		debug_print(TARGET, "BSP: %x %x %x %x %x\n", context->gpr[5], context->gpr[6], context->gpr[7], context->gpr[8], context->gpr[9]);
		debug_print(TARGET, "BSP: %x %x %x\n", context->gpr[10], context->gpr[11], context->gpr[12]);
		debug_print(TARGET, "BSP: sp %x lr %x pc %x cpsr %x\n", context->sp, context->lr, context->pc, context->cpsr);
		debug_prints(TARGET, "BSP: ----------------------\n");
	}
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

mem_t bsp_get_usable_memory_start()
{
	extern uint32_t end_of_the_world;
	return (mem_t)&end_of_the_world;
}

mem_t bsp_get_usable_memory_end()
{
	return (mem_t)((32 * 1024 * 1024) - 1);
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
	asm volatile("hlt");
}
