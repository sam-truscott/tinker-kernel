/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PRINT_OUT_H_
#define PRINT_OUT_H_

#include "time/time_manager.h"

typedef enum debug_subsystem
{
	INITIALISATION = 0,
	SHELL = 1,
	TIMER = 2,
	INTC = 3,
	MEMORY = 4,
	COLLECTIONS = 5,
	PROCESS = 6,
	SCHEDULING = 7,
	SCHEDULING_TRACE = 8,
	SHM = 9,
	TARGET = 10,
	SYSCALL = 11,
	ALARM = 12,
	REGISTRY = 13,
	PIPE = 14,
	PIPE_TRACE = 15,
	IDLE = 16,
	IN = 17,
	ELF_LOADER = 18
} debug_subsystem_t;

void print_set_time_manager(time_manager_t * const tm);

void print_time(void);

void error_print(const char * const msg, ...);

#define debug_print(subsys, msg, ...) debug_print1(subsys, msg, __VA_ARGS__)
#define debug_prints(subsys, msg) debug_print1(subsys, msg, NULL)

void debug_print1(const debug_subsystem_t subsys, const char * const msg, ...);

void printp_out(const char * const msg, ...);

void print_out(const char * const msg);

#endif /* DEBUG_PRINT_H_ */
