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

#include "tinker_api_types.h"
#include "kernel/interrupts/interrupt_manager.h"

void tgt_initialise(void)
{
    // TODO initialise the core system registers
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

#if defined (TARGET_DEBUGGING)
static uint32_t arm_get_sp(void)
{
	uint32_t sp;
	asm("mov %[ps], sp" : [ps]"=r" (sp));
	return sp;
}
#endif

static void arm_bootstrap(thread_entry_point * const entry, uint32_t exit_function, const uint32_t sp) TINKER_API_SUFFIX;
static void __attribute__((naked)) arm_bootstrap(thread_entry_point * const entry, uint32_t exit_function, const uint32_t sp)
{
	asm("mov sp, r2");				/* set the programs stack */
	asm("mov fp, sp");				/* frame pointer */
#if defined(TARGET_DEBUGGING)
	asm("push {r3, r4}");			/* backup two args */
	printp_out("ARM: Mode %d\n", arm_get_cpsr());
	printp_out("ARM: Stack %x\n", arm_get_sp());
	printp_out("ARM: Bootstrap, calling %x\n", entry);
	printp_out("ARM: Bootstrap, exit %x\n", exit_function);
	asm("pop {r3, r4}");			/* restore two args */
#endif
	entry();
	((thread_entry_point*)(exit_function))();
	(void)sp;
}

void tgt_initialise_context(
        const thread_t * thread,
        tgt_context_t ** const context,
        const bool_t kernel_mode,
        const uint32_t exit_function)
{
    uint16_t gpr = 3;
    if (context)
    {
        *context = mem_alloc(process_get_mem_pool(thread_get_parent(thread)), sizeof(tgt_context_t));
        tgt_context_t * const arm_context = *context;
        arm_context->sp = thread_get_virt_stack_base(thread);
        arm_context->gpr[0] = (uint32_t)thread_get_entry_point(thread);
        arm_context->gpr[1] = exit_function;
        arm_context->gpr[2] = arm_context->sp;
        for (; gpr < ARM_CONTEXT_GPR ; gpr++)
        {
            arm_context->gpr[gpr] = 0;
        }
        arm_context->gpr[ARM_FP_REGISTER] = arm_context->sp;
        // TODO initialise the other registers
        // use kernel_mode to set stuff up
        arm_context->lr = (uint32_t)arm_bootstrap;
        if (kernel_mode)
        {
        	arm_context->apsr = PSR_MODE_SUPERVISOR;
        }
        else
        {
        	arm_context->apsr = PSR_MODE_USER;
        }
#if defined(TARGET_DEBUGGING)
    	printp_out("ARM: %x %x %x %x %x\n", arm_context->gpr[0], arm_context->gpr[1], arm_context->gpr[2], arm_context->gpr[3], arm_context->gpr[4]);
    	printp_out("ARM: %x %x %x %x %x\n", arm_context->gpr[5], arm_context->gpr[6], arm_context->gpr[7], arm_context->gpr[8], arm_context->gpr[9]);
    	printp_out("ARM: %x %x %x\n", arm_context->gpr[10], arm_context->gpr[11], arm_context->gpr[12]);
    	printp_out("ARM: sp %x lr %x\n", arm_context->sp, arm_context->lr);
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
        context->gpr[3] = parameter;
        break;
    case 1:
        context->gpr[4] = parameter;
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
	arm_disable_irq();
	arm_disable_fiq();
}

void tgt_enter_usermode(void)
{
#if defined(TARGET_DEBUGGING)
	printp_out("Kernel: Entering user mode\n");
#endif
	arm_set_psr_mode(PSR_MODE_SUPERVISOR);
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
    util_memcpy(thread, context, sizeof(tgt_context_t));
}
