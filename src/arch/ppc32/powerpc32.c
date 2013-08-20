/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "powerpc32.h"
#include "powerpc32_mmu.h"

#include "config.h"
#include "kernel/time/time.h"
#include "kernel/utils/util_memcpy.h"

/**
 * The maximum number of interrupt service routines supported
 */
#define MAX_PPC_IVECT 20

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
	uint32_t gpr_0;
	uint32_t gpr_2_31[PPC_CONTEXT_GPR];
	uint32_t srr0;
	uint32_t srr1;
	uint32_t xer;
	uint32_t cr;
	uint32_t ctr;
	uint64_t fpr[PPC_CONTEXT_FPR]; /* FIXME need to be moved to the end */
	uint32_t lr;
	uint32_t fp;
} __tgt_context_internal_t;
#pragma pack(pop)

/**
 * The interrupt service routine (ISR) table
 */
static __ppc_isr * __ppc_isr_table[MAX_PPC_IVECT];

/**
 * The default, empty, ISR handler
 * @param The saved context from the interrupt
 */
static void __ppc_isr_default_handler(uint32_t vector, __tgt_context_t *, bool_t fp_enabled);

static void __ivt_install_vector(const uint32_t address, uint32_t * vector, uint32_t size);

/**
 * Setup a number of pages in the page table
 * @param vsid The virtual segment ID
 * @param real_address The real address of the page base
 * @param virtual_address The virtual address of the page base
 * @param size The size of the memory area
 * @param rw_access Read/Write access
 * @param mem_type The type of memory being mapped
 */
static error_t __ppc_setup_paged_area(
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec);

static void __ppc_remove_paged_area(
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec);

void __ppc_isr_initialise(void)
{
	uint32_t vect = 0;
	for ( vect = 0 ; vect < MAX_PPC_IVECT ; vect++ )
	{
		__ppc_isr_table[vect] = &__ppc_isr_default_handler;
	}
}

void __ppc_isr_attach(const uint32_t vector, __ppc_isr * isr)
{
	if ( vector < MAX_PPC_IVECT && isr)
	{
		__ppc_isr_table[vector] = isr;
	}
}

__ppc_isr * __ppc_isr_get_isr(const uint32_t vector)
{
	__ppc_isr * the_isr = NULL;

	if ( vector < MAX_PPC_IVECT )
	{
		the_isr = __ppc_isr_table[vector];
	}

	return the_isr;
}

void __ppc_isr_detach(const uint32_t vector)
{
	if ( vector < MAX_PPC_IVECT )
	{
		__ppc_isr_table[vector] = &__ppc_isr_default_handler;
	}
}

void __ppc_isr_default_handler(uint32_t vector, __tgt_context_t * context, bool_t fp_enabled)
{
	/**
	 * TODO log a notificaiton of an unhandled exception
	 */
	if ( vector && context && fp_enabled)
	{

	}
}

uint32_t __tgt_get_syscall_param(const void * context, uint8_t param)
{
	__tgt_context_t * vector = (__tgt_context_t*)context;
	return vector->gpr_2_31[1 + param];
}

void __tgt_set_syscall_return(void * context, uint32_t value)
{
	__tgt_context_t * vector = (__tgt_context_t*)context;
	vector->gpr_2_31[1] = value;
}

void __ppc_get_tbrs(uint32_t * a, uint32_t * b);

uint64_t __ppc_get_tbr(void)
{
	uint32_t tbl = 0, tbu = 0;
	uint64_t tb = 0;
	__ppc_get_tbrs(&tbl, &tbu);
	tb += ( (uint64_t)tbu << 32);
	tb += tbl;
	return tb;
}

uint32_t __ppc_get_ns_per_tb_tick(uint64_t clock_hz, uint32_t ticks_per_clock)
{
	uint64_t ticks_per_second = clock_hz / ticks_per_clock;
	uint32_t ticks_per_ns = (uint32_t)(ONE_SECOND_AS_NANOSECONDS / ticks_per_second);
	return ticks_per_ns;
}

uint32_t __tgt_get_context_stack_pointer(const void * const context)
{
	uint32_t sp = 0;

	__tgt_context_t * const vector = (__tgt_context_t*)context;
	if ( vector )
	{
		sp = vector->sp;
	}

	return sp;
}

#define IVT_SIZE 0xA8u

void __ivt_initialise(void)
{
	uint32_t msr = 0;
	/**
	 * Initialise the ISR table
	 */
	__ppc_isr_initialise();

	__ivt_install_vector(0x100, &__ivt_system_reset_interrupt,IVT_SIZE);
	__ivt_install_vector(0x200, &__ivt_machine_check_interrupt,IVT_SIZE);
	__ivt_install_vector(0x300, &__ivt_data_storage_interrupt,IVT_SIZE);
	__ivt_install_vector(0x400, &__ivt_inst_storage_interrupt,IVT_SIZE);
	__ivt_install_vector(0x500, &__ivt_external_interrupt,IVT_SIZE);
	__ivt_install_vector(0x600, &__ivt_alignment_interrupt,IVT_SIZE);
	__ivt_install_vector(0x700, &__ivt_program_interrupt,IVT_SIZE);
	__ivt_install_vector(0x800, &__ivt_fp_unavailable,IVT_SIZE);
	__ivt_install_vector(0x900, &__ivt_decrementer_interrupt,IVT_SIZE);
	__ivt_install_vector(0xC00, &__ivt_syscall_interrupt,IVT_SIZE);
	__ivt_install_vector(0xD00, &__ivt_trace_interrupt,IVT_SIZE);
	__ivt_install_vector(0xE00, &__ivt_fp_assist_interrupt,IVT_SIZE);

	/**
	 * Enable external interrupts
	 */
	msr = __ppc_get_msr();
	msr |=  MSR_FLAG_EE | MSR_FLAG_RI;
	__ppc_set_msr(msr);
}

void __tgt_enter_usermode(void)
{
	uint32_t msr = __ppc_get_msr();
	msr |=  MSR_FLAG_PR;
	__ppc_set_msr(msr);
}

void __ivt_install_vector(const uint32_t address, uint32_t * vector, uint32_t size)
{
	uint32_t * dst = (uint32_t*)address;
	const uint32_t * end = (uint32_t*) (address + size);
	while(dst < end)
	{
		*dst = *vector;
		dst++;
		vector++;
	}
}

void __ppc_isr_handler(const uint32_t vector, void * registers, bool_t fp_enabled)
{
	__tgt_context_t * const vector_info = (__tgt_context_t*)registers;

	if ( vector_info )
	{
		/* take a copy of the LR incase it's a new thread.
		 * if it's a new thread the LR will be missing and this function will
		 * cause an exception when it returns because it'll jump to
		 * 0x0.
		 * TODO: This should be fixed by the initialise code for a thread
		 * should should set the LR value up correctly to jump to the line
		 * of the ISR code following the interrupt vector.
		 */
		uint32_t tmp_lr = vector_info->restore_lr;
		if (registers)
		{
			__ppc_isr_get_isr(vector)(vector, vector_info, fp_enabled);
		}
		if (vector_info->restore_lr == 0)
		{
			vector_info->restore_lr = tmp_lr;
		}
	}
}

void __tgt_disable_thread_interrupts(__tgt_context_t * const context)
{
	if (context)
	{
		context->srr1 &= (uint32_t)(~MSR_FLAG_EE);
	}
}

static error_t __ppc_setup_paged_area(
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec)
{
	uint32_t w0 = 0;
	uint32_t w1 = 0;

	const uint32_t real_addr = __mem_sec_get_real_addr(mem_sec);
	const uint32_t size = __mem_sec_get_size(mem_sec);
	const uint32_t virt_addr = __mem_sec_get_virt_addr(mem_sec);
	const mmu_memory_t mem_type = __mem_sec_get_mem_type(mem_sec);
	const mmu_access_t access = __mem_sec_get_access(mem_sec);

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
	if ( (size % MMU_PAGE_SIZE) !=0 )
	{
		pages++;
	}

	for ( uint32_t page = 0 ; page < pages ; page++ )
	{
		const uint32_t page_virtual_address = (virt_addr + (page * MMU_PAGE_SIZE));
		const uint32_t page_real_address = (real_addr + (page * MMU_PAGE_SIZE));
		const uint32_t vsid = segment_info->segment_ids[__PPC_GET_SEGMENT_INDEX(page_virtual_address)];

		w0 = __PPC_PTE_W0(
				PTE_VALID,
				vsid,
				HASH_PRIMARY,
				0);

		if ( mem_type == mmu_random_access_memory)
		{
			/* RAM can have cache enabled */
			w1 = __PPC_PTE_W1(
					page_real_address,
					0,
					0,
					__PPC32_WIMG(
							__ppc32_write_back,
							__ppc32_cache_enabled,
							__ppc32_memory_no_coherency,
							__ppc32_not_guarded),
							access);
		}
		else
		{
			/* hardware needs caching disabled and OoO access disabled */
			w1 = __PPC_PTE_W1(
					page_real_address,
					0,
					0,
					__PPC32_WIMG(
							__ppc32_write_through,
							__ppc32_cache_inhibited,
							__ppc32_memory_no_coherency,
							__ppc32_guarded),
							access);
		}
		__ppc32_add_pte(
				page_virtual_address,
				vsid,
				w0,
				w1);
	}

	return true;
}

static void __ppc_remove_paged_area(
		const tgt_mem_t * const segment_info,
		const mem_section_t * const mem_sec)
{
	const uint32_t real_addr = __mem_sec_get_real_addr(mem_sec);
	const uint32_t size = __mem_sec_get_size(mem_sec);
	const uint32_t virt_addr = __mem_sec_get_virt_addr(mem_sec);
	const mmu_memory_t mem_type = __mem_sec_get_mem_type(mem_sec);
	const mmu_access_t access = __mem_sec_get_access(mem_sec);

	uint32_t w0 = 0;
	uint32_t w1 = 0;

	uint32_t pages = size / MMU_PAGE_SIZE;
	if ( (size % MMU_PAGE_SIZE) !=0 )
	{
		pages++;
	}

	for ( uint32_t page = 0 ; page < pages ; page++ )
	{
		const uint32_t page_virtual_address = (virt_addr + (page * MMU_PAGE_SIZE));
		const uint32_t page_real_address = (real_addr + (page * MMU_PAGE_SIZE));
		const uint32_t vsid = segment_info->segment_ids[__PPC_GET_SEGMENT_INDEX(page_virtual_address)];

		w0 = __PPC_PTE_W0(
				PTE_VALID,
				vsid,
				HASH_PRIMARY,
				0);

		if ( mem_type == mmu_random_access_memory)
		{
			/* RAM can have cache enabled */
			w1 = __PPC_PTE_W1(
					page_real_address,
					0,
					0,
					__PPC32_WIMG(
							__ppc32_write_back,
							__ppc32_cache_enabled,
							__ppc32_memory_no_coherency,
							__ppc32_not_guarded),
							access);
		}
		else
		{
			/* hardware needs caching disabled and OoO access disabled */
			w1 = __PPC_PTE_W1(
					page_real_address,
					0,
					0,
					__PPC32_WIMG(
							__ppc32_write_through,
							__ppc32_cache_inhibited,
							__ppc32_memory_no_coherency,
							__ppc32_guarded),
							access);
		}

		__ppc32_remove_pte(
				page_virtual_address,
				vsid,
				w0,
				w1);
	}
}

error_t __tgt_initialise_process(__process_t * const process)
{
	error_t ok = NO_ERROR;

	if ( !__process_is_kernel(process) )
	{
		const uint32_t pid = __process_get_pid(process);
		tgt_mem_t segment_info;
		/* setup all the segment IDs */
		for ( uint8_t sid = 0 ; sid < MMU_SEG_COUNT ; sid++ )
		{
			segment_info.segment_ids[sid] =
					(pid * MMU_SEG_COUNT) + sid;
		}
		__process_set_segment_info(process, &segment_info);

		/* setup pages for all the memory sections, i.e. code, data, rdata, sdata, bss */
		const mem_section_t * section = __process_get_first_section(process);
		while (section && (ok == NO_ERROR))
		{
			/* setup virt -> real mapping for size */
			ok = __ppc_setup_paged_area(
					&segment_info,
					section);

			/* next section */
			section = __mem_sec_get_next(section);
		}
	}

	return ok;
}

void __tgt_destroy_process(const __process_t * const process)
{
	const mem_section_t * section = __process_get_first_section(process);
	const tgt_mem_t * const segment_info = __process_get_segment_info(process);
	while (section)
	{
		__ppc_remove_paged_area(
				segment_info,
				section);

		/* next section */
		section = __mem_sec_get_next(section);
	}
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
		__tgt_context_t * const ppc_context = *context;
		ppc_context->sp = __thread_get_virt_stack_base(thread);
		ppc_context->fp = ppc_context->sp;
		for (uint8_t gpr = 0 ; gpr < PPC_CONTEXT_GPR ; gpr++)
		{
			ppc_context->gpr_2_31[gpr] = 0;
		}
		for ( uint8_t fpr = 0 ; fpr < PPC_CONTEXT_FPR ; fpr++ )
		{
			ppc_context->fpr[fpr] = 0;
		}
		ppc_context->srr0 = (uint32_t)__thread_get_entry_point(thread);

		/* if it's not the kernel we need to add the privilege mode */
		ppc_context->srr1 = MSR_FLAG_ME | MSR_FLAG_RI | MSR_FLAG_EE;
		ppc_context->srr1 |= (MSR_FLAG_IR | MSR_FLAG_DR);
		if ( (__thread_get_flags(thread) & THREAD_FLAG_FP) == THREAD_FLAG_FP)
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

void __tgt_destroy_context(
		const __thread_t * thread,
		__tgt_context_t * const context)
{
	if (context)
	{
		__mem_free(__process_get_mem_pool(__thread_get_parent(thread)), context);
	}
}

void __tgt_prepare_context(
		__tgt_context_t * const context,
		const __thread_t * const thread)
{
	if ( context && thread )
	{
		uint8_t ks_flag = SR_KS_FAIL;
		uint8_t kp_flag = SR_KP_OK;

		const __process_t * const proc = __thread_get_parent(thread);

		if (__process_is_kernel(proc))
		{
			// only the kernel has access to kernel segments
			ks_flag = SR_KS_OK;
		}

		const tgt_mem_t * const segment_info = __process_get_segment_info(proc);

		__thread_load_context(thread, context);

		__ppc32_set_sr0(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[0]));
		__ppc32_set_sr1(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[1]));
		__ppc32_set_sr2(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[2]));
		__ppc32_set_sr3(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[3]));
		__ppc32_set_sr4(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[4]));
		__ppc32_set_sr5(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[5]));
		__ppc32_set_sr6(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[6]));
		__ppc32_set_sr7(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[7]));
		__ppc32_set_sr8(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[8]));
		__ppc32_set_sr9(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[9]));
		__ppc32_set_sr10(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[10]));
		__ppc32_set_sr11(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[11]));
		__ppc32_set_sr12(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[12]));
		__ppc32_set_sr13(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[13]));
		__ppc32_set_sr14(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[14]));
		__ppc32_set_sr15(
				__PPC_SR_T0(
						ks_flag, kp_flag, SR_NE_OFF,
						segment_info->segment_ids[15]));
	}
}

/*
 * These two procedures need to be re-written in
 * assembly but as long as the system remains
 * single process it isn't too bad.
 *
 * TODO switch to atomic check/set operations
 */
void __tgt_acquire_lock(__spinlock_t * lock)
{
	while ( ! *lock )
	{
		*lock = LOCK_ON;
	}
}

void __tgt_release_lock(__spinlock_t * lock)
{
	*lock = LOCK_OFF;
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
