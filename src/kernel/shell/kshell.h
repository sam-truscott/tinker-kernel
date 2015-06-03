/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KSHELL_H_
#define KSHELL_H_

#include "kernel/scheduler/scheduler.h"
#include "kernel/objects/registry.h"
#include "kernel/process/process_list.h"

void kshell_setup(
		mem_pool_info_t * const pool,
		scheduler_t * const scheduler,
		registry_t * const reg,
		proc_list_t * const proc_list);

void kshell_start(void);

#endif /* KSHELL_H_ */
