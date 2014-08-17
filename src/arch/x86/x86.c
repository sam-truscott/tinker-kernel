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

#include "kernel/kernel_initialise.h"
#include "kernel/interrupts/interrupt_manager.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/process/process_manager.h"
#include "kernel/time/time_manager.h"
#include "kernel/time/alarm_manager.h"
#include "kernel/console/print_out.h"
#include "x86_vga.h"
#include "x86_clock.h"
#include "x86_registers.h"
#include "x86_gdt.h"
#include "x86_mode.h"

/**
 * The structure of the saved interrupt vector context
 */
#pragma pack(push,1)
typedef struct tgt_context_t
{
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t oesp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	uint16_t gs;
	const uint16_t padding1;
	uint16_t fs;
	const uint16_t padding2;
	uint16_t es;
	const uint16_t padding3;
	uint16_t ds;
	const uint16_t padding4;
	uint32_t trap_number;

	uint32_t err;
	uint32_t eip;
	uint16_t cs;
	const uint16_t padding5;
	uint32_t eflags;

	uint32_t esp;
	uint16_t ss;
	const uint16_t padding6;
} tgt_context_internal_t;
#pragma pack(pop)

void bsp_initialise(void)
{
	x86_vga_initialise();
	x86_vga_writestring("x86 bsp_initialise\n", 100);
#if defined(KERNEL_DEBUGGING)
	x86_vga_writestring("debugging enabled\n", 100);
#endif // KERNEL_DEBUGGING
	time_set_system_clock(x86_get_ppc_timebase_clock());
	x86_vga_writestring("clock setup\n", 100);

	x86_initialise_gdt();
	x86_enter_protected();
}

void bsp_setup(void)
{
	x86_vga_writestring("x86 bsp_setup\n", 100);
}

void bsp_enable_schedule_timer(void)
{

}

void bsp_check_timers_and_alarms(void)
{

}

uint32_t bsp_get_usable_memory_start()
{
	extern uint32_t _end;
	return (uint32_t)&_end;
}

uint32_t bsp_get_usable_memory_end()
{
	return (128 * 1024 * 1024);
}

void bsp_write_debug_char(const char c)
{
	x86_vga_putchar(c);
}

char bsp_read_debug_char(void)
{
    return 0;
}

error_t tgt_initialise_process(process_t * const process)
{
    error_t ok = NO_ERROR;

    if (!process_is_kernel(process))
    {
        tgt_mem_t segment_info;
        // TODO setup the mem info
        process_set_mem_info(process, &segment_info);

        /* setup pages for all the memory sections, i.e. code, data, rdata, sdata, bss */
        const mem_section_t * section = process_get_first_section(process);
        while (section && (ok == NO_ERROR))
        {
            /* setup virt -> real mapping for size */
            // TODO setup the paging for virtual sections
            // by adding page table entries

            /* next section */
            section = mem_sec_get_next(section);
        }
    }

    return ok;
}

void tgt_initialise_context(
        const thread_t * thread,
        tgt_context_t ** const context,
        const bool_t kernel_mode,
        const uint32_t exit_function)
{
    if (context)
    {
        *context = mem_alloc(process_get_mem_pool(thread_get_parent(thread)), sizeof(tgt_context_t));
        tgt_context_t * const x86_context = *context;
        util_memset(x86_context, 0, sizeof(tgt_context_t));
        x86_context->esp =
        		x86_context->ebp = thread_get_virt_stack_base(thread);
        // TODO initialise the other registers
        // use kernel_mode to set stuff up
        (void)kernel_mode; // UNUSED
        x86_context->eip = exit_function;
    }
}

void tgt_prepare_context(
        tgt_context_t * const context,
        const thread_t * const thread,
        const process_t * const current_process)
{
    if (context && thread)
    {
        const process_t * const proc = thread_get_parent(thread);
        if (process_is_kernel(proc))
        {
            // only the kernel has access to kernel segments
        }

        //const tgt_mem_t * const segment_info = process_get_segment_info(proc);

        thread_load_context(thread, context);

        // TODO: MMU Setup (i.e. segment registers)

        if (current_process != proc) {
            //ppc32_switch_page_table(current_process, proc);
        }
    }
}

void tgt_destroy_context(
        mem_pool_info_t * const pool,
        tgt_context_t * const context)
{
    if (context)
    {
        mem_free(pool, context);
    }
}

uint32_t tgt_get_syscall_param(
        const tgt_context_t * const context,
        const uint8_t param)
{
	const uint32_t * const mem = (const uint32_t*)(context->esp + (4 * param));
    return *mem;
}

void tgt_set_syscall_return(tgt_context_t * const context, const uint32_t value)
{
    *((uint32_t*)(context->esp+4)) = value;
}

void tgt_set_context_param(
        tgt_context_t * const context,
        const uint8_t index,
        const uint32_t parameter)
{
	uint32_t * const mem = (uint32_t*)(context->esp + (4 * index));
	*mem = parameter;
}

error_t tgt_map_memory(
        const process_t * const process,
        const mem_section_t * const section)
{
    error_t result = PARAMETERS_INVALID;
    if (process && section)
    {
        const tgt_mem_t * const segment_info = process_get_mem_info(process);
        if (segment_info)
        {
            //TODO: Map the area
            result = NO_ERROR;
        }
    }
    return result;
}

void tgt_unmap_memory(
        const process_t * const process,
        const mem_section_t * const section)
{
    if (process && section)
    {
        const tgt_mem_t * const segment_info = process_get_mem_info(process);
        if (segment_info)
        {
            // TODO unmap the area
        }
    }
}

void tgt_disable_external_interrupts(void)
{
    // TODO disable inerrupts
}

void tgt_enter_usermode(void)
{
    // TOOD enter usermode
}

uint32_t tgt_get_context_stack_pointer(const tgt_context_t * const context)
{
    uint32_t sp = 0;
    if (context)
    {
        sp = context->esp;
    }
    return sp;
}

uint32_t tgt_get_stack_pointer(void)
{
    // TODO get the stack pointer - need to move this to ASM
    return 0;
}

void tgt_load_context(
        const tgt_context_t * const thread,
        tgt_context_t * const context)
{
    util_memcpy(context, thread, sizeof(tgt_context_t));
}

void tgt_save_context(
        tgt_context_t * const thread,
        const tgt_context_t * const context)
{
    util_memcpy(thread, context, sizeof(tgt_context_t));
}
