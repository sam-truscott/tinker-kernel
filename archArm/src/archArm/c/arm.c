/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arm.h"

#include "config.h"
#include "arm_cpsr.h"
#include "arm_vec.h"
#include "arm_mmu.h"

#include "tinker_api_types.h"
#include "kernel_initialise.h"
#include "interrupts/interrupt_manager.h"
#include "utils/util_memcpy.h"
#include "console/print_out.h"

void tgt_initialise(void)
{
	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "Target: Disable MMU\n");
	}
	arm_disable_mmu();
	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "Target: Invalidate TLBs\n");
	}
	arm_invalidate_all_tlbs();
}

return_t tgt_initialise_process(process_t * const process)
{
    return_t ok = NO_ERROR;

    if (!process_is_kernel(process))
    {
        tgt_mem_t segment_info;
        segment_info.unused = 0;
        process_set_mem_info(process, &segment_info);

        /* setup pages for all the memory sections, i.e. code, data, rdata, sdata, bss */
        const mem_section_t * section = process_get_first_section(process);
        while (section && (ok == NO_ERROR))
        {
            /* setup virt -> real mapping for size */
			tgt_map_memory(process, section);
			if (is_debug_enabled(TARGET))
			{
				debug_print(TARGET, "Mapping %8x to %8x\n", mem_sec_get_virt_addr(section), mem_sec_get_real_addr(section));
			}
            section = mem_sec_get_next(section);
        }
    }

    return ok;
}

#pragma GCC push_options
#pragma GCC optimize ("-O0")

static void __attribute__((naked)) arm_bootstrap(
		thread_entry_point * const entry,
		uint32_t exit_function,
		const uint32_t sp) TINKER_API_SUFFIX;

static void __attribute__((used)) hello_world(void) TINKER_API_SUFFIX;

static void hello_world(void)
{
	char message[] = "STARTIING APP\n";
	tinker_debug(message, 14);
}

static void __attribute__((naked)) arm_bootstrap(
		 /* R0 */ thread_entry_point * const entry,
		 /* R1 */ uint32_t exit_function,
		 /* R2 */ const uint32_t sp)
{
	asm volatile("push {fp, lr}");			/* move the new stack on the stack for the first frame */
	asm volatile("add fp, sp, #4");
	asm volatile("sub sp, sp, #16");
	asm volatile("str r0, [fp, #-8]");		/* these two wrong? */
	asm volatile("str r1, [fp, #-12]");		/* and this? */
	asm volatile("str r2, [fp, #-16]");

	asm volatile("mrs %r7, cpsr");			/* get cpsr */
	asm volatile("mov r8, #0xFFFFFF20");	/* blat out the mode and enable interrupts */
	asm volatile("and r7, r7, r8");			/* and cpsr and mode wipe */
	asm volatile("orr r7, r7, #0x10");		/* set the mode */
	asm volatile("msr cpsr, r7");			/* move the mode into cpsr */

	asm volatile("ldr r0, [fp, #-8]");
	asm volatile("ldr r1, [fp, #-12]");
	asm volatile("ldr r2, [fp, #-16]");

	asm volatile("ldr r3, [fp, #-8]");

	asm volatile("blx r3");					/* call entry 	-> r0 (-8) -> r0 */

	asm volatile("ldr r3, [fp, #-12]");
	asm volatile("blx r3");					/* call exit	-> r1 (-12) -> r3 */

	asm volatile("sub sp, fp, #4");
	asm volatile("pop {fp, lr}");
	(void)sp;
	(void)entry;
	(void)exit_function;
}

#pragma GCC pop_options

void tgt_initialise_context(
        const thread_t * thread,
        tgt_context_t ** const context,
        const uint32_t exit_function)
{
    if (context)
    {
    	process_t * const proc = thread_get_parent(thread);
        *context = mem_alloc(process_get_mem_pool(proc), sizeof(tgt_context_t));
        tgt_context_t * const arm_context = *context;
        for (uint16_t gpr = 0; gpr < ARM_CONTEXT_GPR ; gpr++)
        {
            arm_context->gpr[gpr] = 0;
        }
        arm_context->sp = thread_get_virt_stack_base(thread);
		arm_context->gpr[0] = (uint32_t)thread_get_entry_point(thread);
		arm_context->gpr[1] = exit_function;
		arm_context->gpr[2] = arm_context->sp;
        arm_context->gpr[ARM_FP_REGISTER] = arm_context->sp;
        arm_context->pc = (uint32_t)arm_bootstrap;
        arm_context->lr = 0;
        arm_context->cpsr = PSR_MODE_USER;

        if (is_debug_enabled(TARGET))
        {
			debug_print(TARGET, "ARM: %8x %8x %8x %8x %8x\n", arm_context->gpr[0], arm_context->gpr[1], arm_context->gpr[2], arm_context->gpr[3], arm_context->gpr[4]);
			debug_print(TARGET, "ARM: %8x %8x %8x %8x %8x\n", arm_context->gpr[5], arm_context->gpr[6], arm_context->gpr[7], arm_context->gpr[8], arm_context->gpr[9]);
			debug_print(TARGET, "ARM: %8x %8x %8x\n", arm_context->gpr[10], arm_context->gpr[11], arm_context->gpr[12]);
			debug_print(TARGET, "ARM: sp %8x lr %8x pc %8x\n", arm_context->sp, arm_context->lr, arm_context->pc);
        }
    }
}

void tgt_prepare_context(
        tgt_context_t * const context,
        const thread_t * const thread,
        const process_t * const current_process)
{
    if (context && thread)
    {
        process_t * const proc = thread_get_parent(thread);
        thread_load_context(thread, context);
        if (current_process != proc)
        {
        	tgt_pg_tbl_t * const pg_table = process_get_page_table(proc);
        	if (is_debug_enabled(TARGET))
        	{
        		debug_print(TARGET, "Page table for %s\n", process_get_image(proc));
        	}
        	arm_print_page_table(pg_table);
        	arm_set_translation_table_base(pg_table);
        	arm_invalidate_all_tlbs();
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
        tgt_context_t * const context,
        const uint8_t param)
{
	return context->gpr[param];
}

void tgt_set_syscall_return(tgt_context_t * const context, const uint32_t value)
{
    context->gpr[0] = value;
}

void tgt_set_context_param(
        tgt_context_t * const context,
        const uint8_t index,
        const uint32_t parameter)
{
    switch (index)
    {
    case 0:
        context->gpr[5] = parameter;
        break;
    case 1:
        context->gpr[6] = parameter;
        break;
    default:
    	/* no-op */
    }
}

return_t tgt_map_memory(
        const process_t * const process,
        const mem_section_t * const section)
{
    return_t result = PARAMETERS_INVALID;
    if (process && section)
    {
		arm_map_memory(
				process_get_mem_pool(process),
				process_get_page_table(process),
				section);
		result = NO_ERROR;
    }
    return result;
}

void tgt_unmap_memory(
        const process_t * const process,
        const mem_section_t * const section)
{
    if (process && section)
    {
		arm_unmap_memory(
				process_get_mem_pool(process),
				process_get_page_table(process),
				section);
    }
}

void tgt_disable_external_interrupts(void)
{
	arm_disable_irq();
	arm_disable_fiq();
}

void tgt_enter_usermode(void)
{
	if (is_debug_enabled(TARGET))
	{
		debug_prints(TARGET, "Kernel: Entering user mode\n");
		debug_print(TARGET, "Kernel: CPSR %x\n", arm_get_cpsr());
	}
	arm_enable_irq();
	if (is_debug_enabled(TARGET))
	{
		debug_print(TARGET, "Kernel: CPSR %x\n", arm_get_cpsr());
		debug_prints(TARGET, "Kernel: User mode entered\n");
	}
}

uint32_t tgt_get_context_stack_pointer(const tgt_context_t * const context)
{
    uint32_t sp = 0;
    if (context)
    {
        sp = context->gpr[ARM_FP_REGISTER];
    }
    return sp;
}

uint32_t tgt_get_pc(const tgt_context_t * const context)
{
	uint32_t pc = 0;
	if (context)
	{
		pc = context->pc;
	}
	return pc;
}

uint32_t tgt_get_frame_pointer(void)
{
	uint32_t sp;
	asm volatile("mov %[ps], fp" : [ps]"=r" (sp));
	return sp;
}

uint32_t tgt_get_stack_pointer(void)
{
	uint32_t sp;
	asm volatile("mov %[ps], sp" : [ps]"=r" (sp));
	return sp;
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
	if (thread)
	{
		util_memcpy(thread, context, sizeof(tgt_context_t));
	}
}
