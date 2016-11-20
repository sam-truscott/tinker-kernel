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
	INITIALISATION = 1,
	SHELL = 1 << 1,
	TIMER = 1 << 2,
	INTC = 1 << 3,
	MEMORY = 1 << 4,
	COLLECTIONS = 1 << 5,
	PROCESS = 1 << 6,
	SCHEDULING = 1 << 7,
	SCHEDULING_TRACE = 1 << 8,
	SHM = 1 << 9,
	TARGET = 1 << 10,
	SYSCALL = 1 << 11,
	ALARM = 1 << 12,
	REGISTRY = 1 << 13,
	PIPE = 1 << 14,
	PIPE_TRACE = 1 << 15,
	IDLE = 1 << 16,
	ELF_LOADER = 1 << 17,
	PROCESS_TRACE = 1 << 18
} debug_subsystem_t;

void print_set_time_manager(time_manager_t * const tm);

void print_time(void);

void error_print(const char * const msg, ...);

#define is_debug_enabled(s)(DEBUG_VALUE & s)
#define debug_print(subsys, msg, ...) debug_print1(subsys, __FILE__, __LINE__, msg, __VA_ARGS__)
#define debug_prints(subsys, msg) debug_print(subsys, msg, NULL)

void debug_print1(
		const debug_subsystem_t subsys,
		const char * const file,
		const uint32_t line,
		const char * const msg,
		...);

void printp_out(const char * const msg, ...);

void print_out(const char * const msg);

void print_out_len(const char * const msg, int len);

#endif /* DEBUG_PRINT_H_ */
