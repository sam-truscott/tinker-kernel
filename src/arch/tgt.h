/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_H_
#define TGT_H_

#include "tgt_types.h"
#include "kernel/process/process.h"

void __tgt_initialise(void);

/*
 * Interrupts
 */

void __tgt_enter_usermode(void);

void __tgt_enable_external_interrupts(void);

void __tgt_disable_external_interrupts(void);

error_t __tgt_initialise_process(__process_t * const process);

error_t __tgt_map_memory(
		const __process_t * const process,
		const __mem_section_t * const section);

void __tgt_unmap_memory(
		const __process_t * const process,
		const __mem_section_t * const section);

void __tgt_initialise_context(
		const __thread_t * thread,
		__tgt_context_t ** const context,
		const bool_t kernel_mode,
		const uint32_t exit_function);

void __tgt_destroy_context(
		__mem_pool_info_t * const pool,
		__tgt_context_t * const context);

void __tgt_prepare_context(
		__tgt_context_t * const context,
		const __thread_t * const thread);

uint32_t __tgt_get_stack_pointer(void);

uint32_t __tgt_get_context_stack_pointer(const void * const context);

void __tgt_load_context(
		const __tgt_context_t * const thread,
		__tgt_context_t * const context);

void __tgt_save_context(
		__tgt_context_t * const thread,
		const __tgt_context_t * const context);

void __tgt_set_context_param(
		__tgt_context_t * const context,
		const uint8_t index,
		const uint32_t parameter);
/*
 * Syscall TODO remove
 */
uint32_t __tgt_get_syscall_param(const void * const context, const uint8_t param);

void __tgt_set_syscall_return(void * const context, const uint32_t value);

#endif /* TGT_H_ */
