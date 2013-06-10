/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_H_
#define THT_H_

#include "arch/target_types.h"
#include "kernel/kernel_types.h"

void __tgt_initialise(void);

/*
 * Interrupts
 */

void __tgt_enable_external_interrupts(void);

void __tgt_disable_external_interrupts(void);

void __tgt_disable_thread_interrupts(__thread_t * thread);

void __tgt_initialise_process(__process_t * process);

void __tgt_initialise_context(__thread_t * thread, bool kernel_mode, uint32_t exit_function);

void __tgt_prepare_context(void * context, __thread_t * thread);

uint32_t __tgt_get_stack_pointer(void);

uint32_t __tgt_get_context_stack_pointer(void * context);

/*
 * Syscall TODO remove
 */
uint32_t __tgt_get_syscall_param(const void * context, uint8_t param);

void __tgt_set_syscall_return(void * context, uint32_t value);

/*
 * Locks
 */
void __tgt_acquire_lock(__spinlock_t * lock);

void __tgt_release_lock(__spinlock_t * lock);

#endif /* TGT_H_ */
