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

#include "kernel/interrupts/interrupt_manager.h"

static tgt_context_t arm_vec_handler(arm_vec_t type, tgt_context_t context);

void tgt_initialise(void)
{
    // TODO initialise the core system registers
}

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
    // TODO external interrupts -> int_handle_external_vector();
}

static tgt_context_t arm_vec_handler(arm_vec_t type, tgt_context_t context)
{
	bool_t timer = false;
	printp_out("handler lr %d type %d\n", context.lr, type);
	switch(type)
	{
	case VECTOR_RESET:
	case VECTOR_UNDEFINED:
	case VECTOR_PRETECH_ABORT:
	case VECTOR_DATA_ABORT:
	case VECTOR_RESERVED:
		int_fatal_program_error_interrupt(&context);
		break;
	case VECTOR_SYSTEM_CALL:
		int_syscall_request_interrupt(&context);
		break;
	case VECTOR_IRQ:
	case VECTOR_FIQ:
		if (timer)
		{
			int_context_switch_interrupt(&context);
		}
		else
		{
			int_handle_external_vector();
		}
	}
	return context;
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
        tgt_context_t * const arm_context = *context;
        arm_context->sp = thread_get_virt_stack_base(thread);
        for (uint8_t gpr = 0 ; gpr < ARM_CONTEXT_GPR ; gpr++)
        {
            arm_context->gpr[gpr] = 0;
        }
        // TODO initialise the other registers
        // use kernel_mode to set stuff up
        (void)kernel_mode; // UNUSED
        arm_context->lr = exit_function;
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
        context->gpr[0] = parameter;
        break;
    case 1:
        context->gpr[1] = parameter;
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
    // TODO disable inerrupts
}

void tgt_enter_usermode(void)
{
	printp_out("Entering user mode\n");
	arm_set_psr_mode(PSR_MODE_USER);
	tinker_debug("User mode entered\n");
}

uint32_t tgt_get_context_stack_pointer(const tgt_context_t * const context)
{
    uint32_t sp = 0;
    if (context)
    {
        sp = context->sp;
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
