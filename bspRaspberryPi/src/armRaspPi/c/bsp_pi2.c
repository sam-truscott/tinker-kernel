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

#include "arm.h"
#include "arm_vec.h"
#include "arm_cpsr.h"
#include "arm_mmu.h"

#include "kernel_initialise.h"
#include "kernel_assert.h"
#include "interrupts/interrupt_manager.h"
#include "scheduler/scheduler.h"
#include "process/process_list.h"
#include "time/time_manager.h"
#include "time/alarm_manager.h"
#include "utils/util_memset.h"
#include "bcm2835_timer.h"
#include "bcm2835_uart.h"
#include "bcm2835_intc.h"
#include "shell/kshell.h"

#include "memory/mem_section_private.h"
#include "memory/mem_section.h"

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
#if defined (KERNEL_DEBUGGING)
	const uint32_t cpsr = arm_get_cpsr();
	const uint32_t sctrl = arm_get_cp15_c1();
#endif
	tgt_disable_external_interrupts();
	early_uart_init((void*)0x3f000000);
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
	}

	if (is_debug_enabled(TARGET))
	{
		debug_print(TARGET, "@Startup cpsr is 0x%8x, sctrl = 0x%8x\n", cpsr, sctrl);
		debug_print(TARGET, "SP %8x, FP %8x\n", tgt_get_stack_pointer(), tgt_get_frame_pointer());
	}

	{
		tgt_pg_tbl_t * pg_table = (tgt_pg_tbl_t*)0x02000000;
		util_memset(pg_table, 0, sizeof(tgt_pg_tbl_t));
		mem_section_t sec;
		{
			// map the ram
			debug_prints(TARGET, "RAM\n");
			mem_sec_initialise(
					&sec,
					NULL,
					0,
					0,
					0x08000000,
					MMU_RANDOM_ACCESS_MEMORY,
					MMU_ALL_ACCESS,
					MMU_READ_WRITE,
					"RAM");
			arm_map_memory(NULL, pg_table, &sec);
		}
		{
			// map device space for uart
			debug_prints(TARGET, "RAM (UART)\n");
			mem_sec_initialise(
					&sec,
					NULL,
					0x3f000000,
					0x3f000000,
					0x01000000,
					MMU_DEVICE_MEMORY,
					MMU_ALL_ACCESS,
					MMU_READ_WRITE,
					"RAM (UART)");
			arm_map_memory(NULL, pg_table, &sec);
		}
		if (is_debug_enabled(TARGET))
		{
			debug_print(TARGET, "Setting MMU On, pgtbl @ 0x%8x, cpsr is 0x%8x, sctrl = 0x%8x\n", pg_table, arm_get_cpsr(), arm_get_cp15_c1());
		}
		arm_print_page_table(pg_table);
		arm_set_domain_access_register(0);
		arm_set_translation_table_base(pg_table);
		arm_enable_mmu(false);
		arm_invalidate_all_tlbs();
		if (is_debug_enabled(TARGET))
		{
			early_uart_put("MMU on\n");
		}
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

	arm_print_page_table(process_get_page_table(kernel_process));
	arm_set_translation_table_base(process_get_page_table(kernel_process));
	arm_invalidate_all_tlbs();

	// init uart before setting the page table
	bcm2835_uart_get_device((void*)0x3f000000, &uart, UART_DEVICE_NAME);
#if defined(KERNEL_SHELL)
	// FIXME use object
	kshell_set_input_device(UART_DEVICE_NAME);
#endif

	bcm2835_intc = bcm2835_intc_create(mem_get_default_pool(), (void*)0x3f00B000);
	int_install_isr(controller, bcm2835_intc);

	// we use 1 & 3 as 0 & 2 are reserved for the GPU
	bcm2835_get_timer(mem_get_default_pool(), &bcm2835_scheduler_timer, (void*)0x3f003000, 1);
	bcm2835_get_timer(mem_get_default_pool(), &bcm2835_system_timer, (void*)0x3f003000, 3);

	time_set_system_clock(tm, bcm2835_get_clock((void*)0x3f003000, mem_get_default_pool()));

	alarm_set_timer(am, &bcm2835_system_timer);

	intc_add_timer(bcm2835_intc, INTERRUPT_TIMER1, &bcm2835_scheduler_timer);
	intc_add_timer(bcm2835_intc, INTERRUPT_TIMER3, &bcm2835_system_timer);
	intc_add_device(bcm2835_intc, INTERRUPT_UART, &uart);
	intc_add_device(bcm2835_intc, INTERRUPT_VC_UART, &uart);

	if (is_debug_enabled(TARGET))
	{
		arm_print_page_table(process_get_page_table(kernel_process));
	}

	intc_enable(bcm2835_intc, INTERRUPT_TIMER1);
	intc_enable(bcm2835_intc, INTERRUPT_TIMER3);
	intc_enable(bcm2835_intc, INTERRUPT_VC_UART);
	arm_set_translation_table_base(process_get_page_table(kernel_process));
	arm_invalidate_all_tlbs();

	printp_out("BSP Initialised\n");
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
		error_print("Data Abort sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		break;
	case VECTOR_RESET:
		error_print("Reset sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		break;
	case VECTOR_UNDEFINED:
		error_print("Undefined sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		break;
	case VECTOR_PRETECH_ABORT:
		error_print("Prefetch sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
		break;
	case VECTOR_RESERVED:
		error_print("Reserved sp=[%x] pc=[%x], lr=[%x]\n", context->sp, context->pc, context->lr);
		int_fatal_program_error_interrupt(interrupt_controller, context);
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
	extern uint32_t end;
	return (mem_t)&end;
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
	asm volatile("MOV r0, #0");
	asm volatile("MCR p15, 0, r0, c7, c0, 4");
}
