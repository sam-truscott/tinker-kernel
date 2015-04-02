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
#include "kernel/interrupts/interrupt_manager.h"

void tgt_initialise(void)
{
	arm_disable_mmu();
	arm_invalidate_all_tlbs();
}

error_t tgt_initialise_process(process_t * const process)
{
    error_t ok = NO_ERROR;

    if (!process_is_kernel(process))
    {
        tgt_mem_t segment_info;
        segment_info.unused = 0;
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

static void __attribute__((naked)) arm_bootstrap(thread_entry_point * const entry, uint32_t exit_function, const uint32_t sp)
{
	asm("mov sp, r2");				/* set the programs stack */
	asm("push {fp, lr}");
	asm("mov r0, r5");				/* set arg1 */
	asm("mov r1, r6");				/* set arg2 */
	entry();
	((thread_entry_point*)(exit_function))();
	asm("pop {fp, lr}");
	(void)sp;
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
        tgt_context_t * const arm_context = *context;
        for (uint16_t gpr = 0; gpr < ARM_CONTEXT_GPR ; gpr++)
        {
            arm_context->gpr[gpr] = 0;
        }
        arm_context->alignment = 0;
        arm_context->sp = thread_get_virt_stack_base(thread);
		arm_context->gpr[0] = (uint32_t)thread_get_entry_point(thread);
		arm_context->gpr[1] = exit_function;
		arm_context->gpr[2] = arm_context->sp;
        arm_context->gpr[ARM_FP_REGISTER] = arm_context->sp;
        arm_context->usr_lr = arm_context->lr = (uint32_t)arm_bootstrap;
        if (kernel_mode)
        {
        	arm_context->apsr = PSR_MODE_SYSTEM;
        }
        else
        {
        	arm_context->apsr = PSR_MODE_USER;
        }
#if defined(TARGET_DEBUGGING)
        debug_print("ARM: %x %x %x %x %x\n", arm_context->gpr[0], arm_context->gpr[1], arm_context->gpr[2], arm_context->gpr[3], arm_context->gpr[4]);
        debug_print("ARM: %x %x %x %x %x\n", arm_context->gpr[5], arm_context->gpr[6], arm_context->gpr[7], arm_context->gpr[8], arm_context->gpr[9]);
        debug_print("ARM: %x %x %x\n", arm_context->gpr[10], arm_context->gpr[11], arm_context->gpr[12]);
        debug_print("ARM: sp %x lr %x\n", arm_context->sp, arm_context->lr);
#endif
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
        if (current_process != proc) {
        	arm_set_translation_table_base(process_get_page_table(proc));
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
    switch (index) {
    case 0:
        context->gpr[5] = parameter;
        break;
    case 1:
        context->gpr[6] = parameter;
        break;
    }
}

error_t tgt_map_memory(
        const process_t * const process,
        const mem_section_t * const section)
{
    error_t result = PARAMETERS_INVALID;
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
#if defined(TARGET_DEBUGGING)
	printp_out("Kernel: Entering user mode\n");
#endif
	arm_set_psr_mode(PSR_MODE_SYSTEM);
	arm_enable_irq();
#if defined(TARGET_DEBUGGING)
	tinker_debug("Kernel: User mode entered\n");
#endif
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

uint32_t tgt_get_frame_pointer(void)
{
	uint32_t sp;
	asm("mov %[ps], fp" : [ps]"=r" (sp));
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
