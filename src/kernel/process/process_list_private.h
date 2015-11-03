/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_PROCESS_PROCESS_LIST_PRIVATE_H_
#define KERNEL_PROCESS_PROCESS_LIST_PRIVATE_H_

#include "config.h"
#include "arch/tgt.h"
#include "kernel/scheduler/scheduler.h"

UNBOUNDED_LIST_INTERNAL_TYPE(process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_CREATE(static, process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_INITIALISE(static, process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_GET(static, process_list_t, process_t*)
UNBOUNDED_LIST_SPEC_ADD(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_CREATE(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_INITIALISE(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_GET(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_ADD(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_REMOVE(static, process_list_t, process_t*)
UNBOUNDED_LIST_BODY_REMOVE_ITEM(static, process_list_t, process_t*)

typedef struct proc_list_t
{
	scheduler_t * scheduler;
	process_list_t * process_list;
	alarm_manager_t * alarm_manager;
	uint32_t last_pid;
	process_t * kernel_process;
} proc_list_t;

#endif /* KERNEL_PROCESS_PROCESS_LIST_PRIVATE_H_ */
