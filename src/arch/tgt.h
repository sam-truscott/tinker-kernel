/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_H_
#define TGT_H_

#include "tgt_types.h"
#include "kernel/process/process.h"

void tgt_initialise(void);

/*
 * Interrupts
 */

void tgt_enter_usermode(void);

void tgt_disable_external_interrupts(void);

error_t tgt_initialise_process(process_t * const process);

error_t tgt_map_memory(
		const process_t * const process,
		const mem_section_t * const section);

void tgt_unmap_memory(
		const process_t * const process,
		const mem_section_t * const section);

void tgt_initialise_context(
		const thread_t * thread,
		tgt_context_t ** const context,
		const bool_t kernel_mode,
		const uint32_t exit_function);

void tgt_destroy_context(
		mem_pool_info_t * const pool,
		tgt_context_t * const context);

void tgt_prepare_context(
		tgt_context_t * const context,
		const thread_t * const thread,
        const process_t * const current_process);

uint32_t tgt_get_frame_pointer(void);

uint32_t tgt_get_context_stack_pointer(const tgt_context_t * const context);

void tgt_load_context(
		const tgt_context_t * const thread,
		tgt_context_t * const context);

void tgt_save_context(
		tgt_context_t * const thread,
		const tgt_context_t * const context);

uint32_t tgt_get_syscall_param(
		const tgt_context_t * const context,
		const uint8_t param);

void tgt_set_context_param(
		tgt_context_t * const context,
		const uint8_t index,
		const uint32_t parameter);

void tgt_set_syscall_return(
        tgt_context_t * const context,
		const uint32_t value);

#endif /* TGT_H_ */
