/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "armv6.h"

#include "config.h"

#define ARM_CONTEXT_GPR 13

/**
 * The structure of the saved interrupt vector context
 */
#pragma pack(push,1)
typedef struct __tgt_context_t
{
    uint32_t sp;
    uint32_t restore_lr;
    uint32_t link_area_1;
    uint32_t link_area_2;
    uint32_t gpr[ARM_CONTEXT_GPR];
    uint32_t apsr;
    uint32_t lr;
} __tgt_context_internal_t;
#pragma pack(pop)

void __tgt_initialise(void)
{
    // TODO initialise the core system registers
}

void __ivt_initialise(void)
{
    // TODO write values into the vector table
    // TODO external interrupts -> __int_handle_external_vector();
}

error_t __tgt_initialise_process(__process_t * const process)
{
    error_t ok = NO_ERROR;

    if (!__process_is_kernel(process))
    {
        tgt_mem_t segment_info;
        // TODO setup the mem info
        __process_set_mem_info(process, &segment_info);

        /* setup pages for all the memory sections, i.e. code, data, rdata, sdata, bss */
        const __mem_section_t * section = __process_get_first_section(process);
        while (section && (ok == NO_ERROR))
        {
            /* setup virt -> real mapping for size */
            // TODO setup the paging for virtual sections
            // by adding page table entries

            /* next section */
            section = __mem_sec_get_next(section);
        }
    }

    return ok;
}

void __tgt_initialise_context(
        const __thread_t * thread,
        __tgt_context_t ** const context,
        const bool_t kernel_mode,
        const uint32_t exit_function)
{
    if (context)
    {
        *context = __mem_alloc(__process_get_mem_pool(__thread_get_parent(thread)), sizeof(__tgt_context_t));
        __tgt_context_t * const arm_context = *context;
        arm_context->sp = __thread_get_virt_stack_base(thread);
        for (uint8_t gpr = 0 ; gpr < ARM_CONTEXT_GPR ; gpr++)
        {
            arm_context->gpr[gpr] = 0;
        }
        // TODO initialise the other registers
        // use kernel_mode to set stuff up
        (void)kernel_mode; // UNUSED
        arm_context->lr = exit_function;
        arm_context->restore_lr = 0;
    }
}

void __tgt_prepare_context(
        __tgt_context_t * const context,
        const __thread_t * const thread,
        const __process_t * const current_process)
{
    if (context && thread)
    {
        const __process_t * const proc = __thread_get_parent(thread);
        if (__process_is_kernel(proc))
        {
            // only the kernel has access to kernel segments
        }

        //const tgt_mem_t * const segment_info = __process_get_segment_info(proc);

        __thread_load_context(thread, context);

        // TODO: MMU Setup (i.e. segment registers)

        if (current_process != proc) {
            //__ppc32_switch_page_table(current_process, proc);
        }
    }
}

void __tgt_destroy_context(
        __mem_pool_info_t * const pool,
        __tgt_context_t * const context)
{
    if (context)
    {
        __mem_free(pool, context);
    }
}

uint32_t __tgt_get_syscall_param(
        const __tgt_context_t * const context,
        const uint8_t param)
{
    return context->gpr[param];
}

void __tgt_set_syscall_return(__tgt_context_t * const context, const uint32_t value)
{
    context->gpr[0] = value;
}

void __tgt_set_context_param(
        __tgt_context_t * const context,
        const uint8_t index,
        const uint32_t parameter)
{
    switch (index) {
    case 0:
        context->gpr[0] = parameter;
        break;
    case 1:
        context->gpr[1] = parameter;
        break;
    }
}

error_t __tgt_map_memory(
        const __process_t * const process,
        const __mem_section_t * const section)
{
    error_t result = PARAMETERS_INVALID;
    if (process && section)
    {
        const tgt_mem_t * const segment_info = __process_get_mem_info(process);
        if (segment_info)
        {
            //TODO: Map the area
            result = NO_ERROR;
        }
    }
    return result;
}

void __tgt_unmap_memory(
        const __process_t * const process,
        const __mem_section_t * const section)
{
    if (process && section)
    {
        const tgt_mem_t * const segment_info = __process_get_mem_info(process);
        if (segment_info)
        {
            // TODO unmap the area
        }
    }
}

void __tgt_disable_external_interrupts(void)
{
    // TODO disable inerrupts
}

void __tgt_enter_usermode(void)
{
    // TOOD enter usermode
}

uint32_t __tgt_get_context_stack_pointer(const __tgt_context_t * const context)
{
    uint32_t sp = 0;
    if (context)
    {
        sp = context->sp;
    }
    return sp;
}

uint32_t __tgt_get_stack_pointer(void)
{
    // TODO get the stack pointer - need to move this to ASM
    return 0;
}

void __tgt_load_context(
        const __tgt_context_t * const thread,
        __tgt_context_t * const context)
{
    __util_memcpy(context, thread, sizeof(__tgt_context_t));
}

void __tgt_save_context(
        __tgt_context_t * const thread,
        const __tgt_context_t * const context)
{
    __util_memcpy(thread, context, sizeof(__tgt_context_t));
}
