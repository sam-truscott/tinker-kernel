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
#include "tgt.h"
#include "scheduler/scheduler.h"

typedef struct proc_list_t
{
	scheduler_t * scheduler;
	list_t * process_list;
	alarm_manager_t * alarm_manager;
	uint32_t last_pid;
	process_t * kernel_process;
} proc_list_t;

#endif /* KERNEL_PROCESS_PROCESS_LIST_PRIVATE_H_ */
