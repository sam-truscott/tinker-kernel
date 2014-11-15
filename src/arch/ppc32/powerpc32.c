/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "powerpc32.h"
#include "powerpc32_mmu.h"

#include "config.h"
#include "api/tinker_api_time.h"
#include "kernel/time/time.h"
#include "kernel/utils/util_memcpy.h"
#include "kernel/kernel_assert.h"

/**
 * The maximum number of interrupt service routines supported
 */
#define MAX_PPC_IVECT 20

/**
 * The structure of the saved interrupt vector context
 */
#pragma pack(push,1)
typedef struct tgt_context_t
{
	uint32_t sp;
	uint32_t restore_lr;
	uint32_t link_area_1;
	uint32_t link_area_2;
	uint32_t gpr_0;
	uint32_t gpr_2_31[PPC_CONTEXT_GPR];
	uint32_t srr0;
	uint32_t srr1;
	uint32_t xer;
	uint32_t cr;
	uint32_t ctr;
#if defined(KERNEL_POWERPC_FPS)
	uint64_t fpr[PPC_CONTEXT_FPR];
#endif
	uint32_t lr;
	uint32_t fp;
} tgt_context_internal_t;
#pragma pack(pop)

/**
 * The interrupt service routine (ISR) table
 */
static ppc_isr * ppc_isr_table[MAX_PPC_IVECT];

/**
 * The default, empty, ISR handler
 * @param The saved context from the interrupt
 */
static void ppc_isr_default_handler(const uint32_t vector, tgt_context_t * const context);

static void ivt_install_vector(void * const address, const void * const vector, const uint32_t size);

/**
 * Setup a number of pages in the page table
 * @param vsid The virtual segment ID
 * @param real_address The real address of the page base
 * @param virtual_address The virtual address of the page base
 * @param size The size of the memory area
 * @param rw_access Read/Write access
 * @param mem_type The type of memory being mapped
 */
static error_t ppc_setup_paged_area(
		const tgt_pg_tbl_t * const page_tbl,
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec);

static void ppc_remove_paged_area(
		const tgt_pg_tbl_t * const page_tbl,
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec);

void ppc_isr_initialise(void)
{
	uint32_t vect = 0;
	for ( vect = 0 ; vect < MAX_PPC_IVECT ; vect++ )
	{
		ppc_isr_table[vect] = &ppc_isr_default_handler;
	}
}

void ppc_isr_attach(const uint32_t vector, ppc_isr * const isr)
{
	if ( vector < MAX_PPC_IVECT && isr)
	{
		ppc_isr_table[vector] = isr;
	}
}

ppc_isr * ppc_isr_get_isr(const uint32_t vector)
{
	ppc_isr * the_isr = NULL;

	if ( vector < MAX_PPC_IVECT )
	{
		the_isr = ppc_isr_table[vector];
	}

	return the_isr;
}

void ppc_isr_default_handler(const uint32_t vector, tgt_context_t * const context)
{
	if (context)
	{
		debug_print("Unexpected exception: isr %d\n", vector);
	}
}

uint32_t tgt_get_syscall_param(
		const tgt_context_t * const context,
		const uint8_t param)
{
	return context->gpr_2_31[1 + param];
}

void tgt_set_syscall_return(tgt_context_t * const context, const uint32_t value)
{
	context->gpr_2_31[1] = value;
}

void ppc_get_tbrs(uint32_t * const a, uint32_t * const b);

uint64_t ppc_get_tbr(void)
{
	uint32_t tbl = 0, tbu = 0;
	uint64_t tb = 0;
	ppc_get_tbrs(&tbl, &tbu);
	tb += ( (uint64_t)tbu << 32);
	tb += tbl;
	return tb;
}

uint32_t ppc_get_ns_per_tb_tick(const uint64_t * const clock_hz, const uint32_t ticks_per_clock)
{
	uint64_t ticks_per_second = (*clock_hz) / ticks_per_clock;
	uint32_t ticks_per_ns = (ONE_SECOND_AS_NANOSECONDS / ticks_per_second);
	return ticks_per_ns;
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

#define IVT_SIZE 0xA8u

void ivt_initialise(void)
{
	uint32_t msr = 0;
	/**
	 * Initialise the ISR table
	 */
	ppc_isr_initialise();

	ivt_install_vector((void*)0x100, &ivt_system_reset_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x200, &ivt_machine_check_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x300, &ivt_data_storage_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x400, &ivt_inst_storage_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x500, &ivt_external_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x600, &ivt_alignment_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x700, &ivt_program_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0x800, &ivt_fp_unavailable,IVT_SIZE);
	ivt_install_vector((void*)0x900, &ivt_decrementer_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0xC00, &ivt_syscall_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0xD00, &ivt_trace_interrupt,IVT_SIZE);
	ivt_install_vector((void*)0xE00, &ivt_fp_assist_interrupt,IVT_SIZE);

	/**
	 * Enable external interrupts
	 */
	msr = ppc_get_msr();
	msr |=  MSR_FLAG_EE | MSR_FLAG_RI;
	ppc_set_msr(msr);
}

void tgt_enter_usermode(void)
{
	uint32_t msr = ppc_get_msr();
	msr |=  MSR_FLAG_PR;
	ppc_set_msr(msr);
}

void ivt_install_vector(void * const address, const void * const vector, const uint32_t size)
{
	util_memcpy(address, vector, size);
}

void ppc_isr_handler(const uint32_t vector, tgt_context_t * const context)
{
	if (context)
	{
		/* take a copy of the LR incase it's a new thread.
		 * if it's a new thread the LR will be missing and this function will
		 * cause an exception when it returns because it'll jump to
		 * 0x0.
		 */
		const uint32_t tmp_lr = context->restore_lr;
        ppc_isr * const isr = ppc_isr_get_isr(vector);
        if (isr)
        {
            isr(vector, context);
        }
		if (context->restore_lr == 0)
		{
			context->restore_lr = tmp_lr;
		}
	}
}

static error_t ppc_setup_paged_area(
		const tgt_pg_tbl_t * const page_tbl,
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec)
{
	uint32_t w0 = 0;
	uint32_t w1 = 0;

	const uint32_t real_addr = mem_sec_get_real_addr(mem_sec);
	const uint32_t size = mem_sec_get_size(mem_sec);
	const uint32_t virt_addr = mem_sec_get_virt_addr(mem_sec);
	const mmu_memory_t mem_type = mem_sec_get_mem_type(mem_sec);
	const mmu_access_t access = mem_sec_get_access(mem_sec);

	if ((real_addr % MMU_PAGE_SIZE) != 0)
	{
		return MEM_NOT_ALIGNED;
	}
	if ((size % MMU_PAGE_SIZE) != 0)
	{
		return MEM_NOT_ALIGNED;
	}
	if ((virt_addr % MMU_PAGE_SIZE) != 0)
	{
		return MEM_NOT_ALIGNED;
	}

	uint32_t pages = size / MMU_PAGE_SIZE;
	if ((size % MMU_PAGE_SIZE) !=0)
	{
		pages++;
	}

	for (uint32_t page = 0 ; page < pages ; page++)
	{
		const uint32_t page_virtual_address = (virt_addr + (page * MMU_PAGE_SIZE));
		const uint32_t page_real_address = (real_addr + (page * MMU_PAGE_SIZE));
		const uint32_t vsid = segment_info->segment_ids[PPC_GET_SEGMENT_INDEX(page_virtual_address)];

		w0 = PPC_PTE_W0(
				PTE_VALID,
				vsid,
				HASH_PRIMARY,
				0);

		if (mem_type == MMU_RANDOM_ACCESS_MEMORY)
		{
			/* RAM can have cache enabled */
			w1 = PPC_PTE_W1(
					page_real_address,
					1,
					0,
					PPC32_WIMG(
							ppc32_write_back,
							ppc32_cache_enabled,
							ppc32_memory_no_coherency,
							ppc32_not_guarded),
							access);
		}
		else
		{
			/* hardware needs caching disabled and OoO access disabled */
			w1 = PPC_PTE_W1(
					page_real_address,
					1,
					0,
					PPC32_WIMG(
							ppc32_write_through,
							ppc32_cache_inhibited,
							ppc32_memory_no_coherency,
							ppc32_guarded),
							access);
		}
		ppc32_add_pte(
				page_tbl,
				page_virtual_address,
				vsid,
				w0,
				w1);
	}

	return true;
}

static void ppc_remove_paged_area(
		const tgt_pg_tbl_t * const page_tbl,
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec)
{
	const uint32_t real_addr = mem_sec_get_real_addr(mem_sec);
	const uint32_t size = mem_sec_get_size(mem_sec);
	const uint32_t virt_addr = mem_sec_get_virt_addr(mem_sec);
	const mmu_memory_t mem_type = mem_sec_get_mem_type(mem_sec);
	const mmu_access_t access = mem_sec_get_access(mem_sec);

	uint32_t w0 = 0;
	uint32_t w1 = 0;

	uint32_t pages = size / MMU_PAGE_SIZE;
	if ((size % MMU_PAGE_SIZE) !=0)
	{
		pages++;
	}

	for (uint32_t page = 0 ; page < pages ; page++)
	{
		const uint32_t page_virtual_address = (virt_addr + (page * MMU_PAGE_SIZE));
		const uint32_t page_real_address = (real_addr + (page * MMU_PAGE_SIZE));
		const uint32_t vsid = segment_info->segment_ids[PPC_GET_SEGMENT_INDEX(page_virtual_address)];

		w0 = PPC_PTE_W0(
				PTE_VALID,
				vsid,
				HASH_PRIMARY,
				0);

		if (mem_type == MMU_RANDOM_ACCESS_MEMORY)
		{
			/* RAM can have cache enabled */
			w1 = PPC_PTE_W1(
					page_real_address,
					1,
					0,
					PPC32_WIMG(
							ppc32_write_back,
							ppc32_cache_enabled,
							ppc32_memory_no_coherency,
							ppc32_not_guarded),
							access);
		}
		else
		{
			/* hardware needs caching disabled and OoO access disabled */
			w1 = PPC_PTE_W1(
					page_real_address,
					1,
					0,
					PPC32_WIMG(
							ppc32_write_through,
							ppc32_cache_inhibited,
							ppc32_memory_no_coherency,
							ppc32_guarded),
							access);
		}

		ppc32_remove_pte(
				page_tbl,
				page_virtual_address,
				vsid,
				w0,
				w1);
	}
}

error_t tgt_initialise_process(process_t * const process)
{
	error_t ok = NO_ERROR;

	if (!process_is_kernel(process))
	{
		tgt_mem_t segment_info;
		/* setup all the segment IDs */
		for (uint8_t sid = 0 ; sid < MMU_SEG_COUNT ; sid++)
		{
			segment_info.segment_ids[sid] = sid;
		}
		process_set_mem_info(process, &segment_info);

		/* setup pages for all the memory sections, i.e. code, data, rdata, sdata, bss */
		const mem_section_t * section = process_get_first_section(process);
		while (section && (ok == NO_ERROR))
		{
			/* setup virt -> real mapping for size */
			ok = ppc_setup_paged_area(
			        process_get_page_table(process),
					&segment_info,
					section);

			/* next section */
			section = mem_sec_get_next(section);
		}
	}

	return ok;
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
			result = ppc_setup_paged_area(
			        process_get_page_table(process),
					segment_info,
					section);
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
			ppc_remove_paged_area(
					process_get_page_table(process),
					segment_info,
					section);
		}
	}
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
		tgt_context_t * const ppc_context = *context;
		ppc_context->sp = thread_get_virt_stack_base(thread);
		ppc_context->fp = ppc_context->sp;
		for (uint8_t gpr = 0 ; gpr < PPC_CONTEXT_GPR ; gpr++)
		{
			ppc_context->gpr_2_31[gpr] = 0;
		}
#if defined(KERNEL_POWERPC_FPS)
		for ( uint8_t fpr = 0 ; fpr < PPC_CONTEXT_FPR ; fpr++ )
		{
			ppc_context->fpr[fpr] = 0;
		}
#endif
		ppc_context->srr0 = (uint32_t)thread_get_entry_point(thread);

		/* if it's not the kernel we need to add the privilege mode */
		ppc_context->srr1 = MSR_FLAG_ME | MSR_FLAG_RI | MSR_FLAG_EE;
		ppc_context->srr1 |= (MSR_FLAG_IR | MSR_FLAG_DR);
		if ( (thread_get_flags(thread) & THREAD_FLAG_FP) == THREAD_FLAG_FP)
		{
			ppc_context->srr1 |= MSR_FLAG_FP;
		}
		if ( kernel_mode == false )
		{
			/* kernel mode code runs as supervisor */
			ppc_context->srr1 |= MSR_FLAG_PR;
		}

		ppc_context->xer = 0;
		ppc_context->cr = 0;
		ppc_context->ctr = 0;
		ppc_context->lr = exit_function;
		ppc_context->restore_lr = 0;
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

void tgt_prepare_context(
		tgt_context_t * const context,
		const thread_t * const thread,
		const process_t * const current_process)
{
	if (context && thread)
	{
		uint8_t ks_flag = SR_KS_FAIL;
		uint8_t kp_flag = SR_KP_OK;

		const process_t * const proc = thread_get_parent(thread);

		if (process_is_kernel(proc))
		{
			// only the kernel has access to kernel segments
			ks_flag = SR_KS_OK;
		}

		const tgt_mem_t * const segment_info = process_get_mem_info(proc);

		thread_load_context(thread, context);

		ppc32_set_sr0(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[0]));
		ppc32_set_sr1(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[1]));
		ppc32_set_sr2(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[2]));
		ppc32_set_sr3(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[3]));
		ppc32_set_sr4(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[4]));
		ppc32_set_sr5(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[5]));
		ppc32_set_sr6(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[6]));
		ppc32_set_sr7(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[7]));
		ppc32_set_sr8(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[8]));
		ppc32_set_sr9(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[9]));
		ppc32_set_sr10(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[10]));
		ppc32_set_sr11(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[11]));
		ppc32_set_sr12(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[12]));
		ppc32_set_sr13(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[13]));
		ppc32_set_sr14(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[14]));
		ppc32_set_sr15(
				PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[15]));

		if (current_process != proc) {
		    ppc32_switch_page_table(current_process, proc);
		}
	}
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

void tgt_set_context_param(
		tgt_context_t * const context,
		const uint8_t index,
		const uint32_t parameter)
{
	switch (index) {
	case 0:
		context->gpr_2_31[1] = parameter;
		break;
	case 1:
		context->gpr_2_31[2] = parameter;
		break;
	}
}
