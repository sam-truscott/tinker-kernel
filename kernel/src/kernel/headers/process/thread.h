/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef THREAD_H_
#define THREAD_H_

#include "tinker_api_process.h"
#include "memory/mem_pool.h"

typedef uint8_t priority_t;

typedef struct thread_t thread_t;

typedef struct fwd_process_t fwd_process_t;

thread_t * thread_create(
		mem_pool_info_t * const pool,
		const fwd_process_t * const parent,
		const priority_t priority,
		thread_entry_point * entry_point,
		const uint32_t flags,
		const uint32_t stack,
		const char * const name);

uint32_t thread_get_tid(const thread_t * const thread);

const char * thread_get_name(const thread_t * const thread);

uint32_t thread_get_stack_size(const thread_t * const thread);

void thread_set_tid(
		thread_t * const thread,
		const uint32_t tid);

void thread_set_oid(
		thread_t * const thread,
		const object_number_t oid);

struct process_t * thread_get_parent(
		const thread_t * const thread);

thread_state_t thread_get_state(
		const thread_t * const thread);

void thread_set_state(
		thread_t * const thread,
		const thread_state_t new_state);

priority_t thread_get_priority(
		const thread_t * const thread);

void thread_set_priority(
		thread_t * const thread,
		const priority_t priority);

void thread_load_context(
		const thread_t * const thread,
		tgt_context_t * const context);

void thread_save_context(
		thread_t * const thread,
		const tgt_context_t * const context);

void thread_set_context_param(
		thread_t * const thread,
		const uint8_t index,
		const uint32_t parameter);

object_number_t thread_get_object_no(
		const thread_t * const thread);

uint32_t thread_get_virt_stack_base(
		const thread_t * const thread);

const tgt_context_t * thread_get_context(
		const thread_t * const thread);

thread_entry_point * thread_get_entry_point(
		const thread_t * const thread);

uint32_t thread_get_flags(
		const thread_t * const thread);

const struct object_t * thread_get_waiting_on(
		const thread_t * const thread);

void thread_set_waiting_on(
		thread_t * const thread,
		const struct object_t * const object);

void thread_exit(thread_t * const thread);

#endif /* THREAD_H_ */
