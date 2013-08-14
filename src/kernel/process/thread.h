/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef THREAD_H_
#define THREAD_H_

#include "sos_api_process.h"
#include "kernel/memory/mem_pool.h"

typedef uint8_t priority_t;

typedef struct __thread_t __thread_t;

typedef struct __fwd_process_t  __fwd_process_t;

__thread_t * __thread_create(
		__mem_pool_info_t * const pool,
		const __fwd_process_t * const parent,
		const priority_t priority,
		thread_entry_point * entry_point,
		const uint32_t flags,
		const uint32_t stack,
		const char * const name);

uint32_t __thread_get_tid(const __thread_t * const thread);

const char * __thread_get_name(const __thread_t * const thread);

void __thread_set_tid(
		__thread_t * const thread,
		const uint32_t tid);

void __thread_set_oid(
		__thread_t * const thread,
		const object_number_t oid);

struct __process_t * __thread_get_parent(
		const __thread_t * const thread);

__thread_state_t __thread_get_state(
		const __thread_t * const thread);

void __thread_set_state(
		__thread_t * const thread,
		const __thread_state_t new_state);

uint32_t __thread_get_stack_size(
		const __thread_t * const thread);

priority_t __thread_get_priority(
		const __thread_t * const thread);

void __thread_set_priority(
		__thread_t * const thread,
		const priority_t priority);

void __thread_load_context(
		const __thread_t * const thread,
		__tgt_context_t * const context);

void __thread_save_context(
		__thread_t * const thread,
		const __tgt_context_t * const context);

object_number_t __thread_get_object_no(
		const __thread_t * const thread);

void * __thread_get_stack_memory(
		const __thread_t * const thread);

uint32_t __thread_get_virt_stack_base(
		const __thread_t * const thread);

uint32_t __thread_get_real_stack_base(
		const __thread_t * const thread);

thread_entry_point * __thread_get_entry_point(
		const __thread_t * const thread);

uint32_t __thread_get_flags(
		const __thread_t * const thread);

const struct __object_t * __thread_get_waiting_on(
		const __thread_t * const thread);

void __thread_set_waiting_on(
		__thread_t * const thread,
		const struct __object_t * const object);

void __thread_exit(__thread_t * const thread);

#endif /* THREAD_H_ */
