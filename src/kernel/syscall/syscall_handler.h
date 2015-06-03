/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SYSCALL_HANDLER_H_
#define SYSCALL_HANDLER_H_

#include "arch/tgt_types.h"
#include "kernel/memory/memory_manager.h"
#include "kernel/process/process_list.h"
#include "kernel/objects/registry.h"

typedef struct syscall_handler_t syscall_handler_t;

syscall_handler_t * create_handler(
		mem_pool_info_t * const pool,
		proc_list_t * const proc_list,
		registry_t * const reg,
		scheduler_t * const scheduler,
		time_manager_t * const tm);

void syscall_handle_system_call(
		syscall_handler_t * const handler,
		tgt_context_t * const context);

#endif /* SYSCALL_HANDLER_H_ */
