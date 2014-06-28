/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_PROCESS_H_
#define TINKER_API_PROCESS_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"
#include "tinker_api_kernel_interface.h"

/**
 * The entry point for a new thread
 */
typedef void (thread_entry_point)(void);

/**
 * The type in user space for a process
 */
typedef object_number_t tinker_process_t;

/**
 * The type in user space for a thread
 */
typedef object_number_t tinker_thread_t;

typedef struct tinker_meminfo {
	uint32_t heap_size;
	uint32_t stack_size;
	uint32_t text_start;
	uint32_t text_size;
	uint32_t data_start;
	uint32_t data_size;
} tinker_meminfo_t;

error_t tinker_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		const tinker_meminfo_t * const meminfo,
		uint32_t flags,
		tinker_process_t * process) TINKER_API_SUFFIX;

error_t tinker_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		tinker_thread_t * thread) TINKER_API_SUFFIX;

error_t tinker_get_thread_object(tinker_thread_t * thread) TINKER_API_SUFFIX ;

error_t	tinker_get_thread_priority(
		tinker_thread_t thread,
		uint8_t * priority) TINKER_API_SUFFIX;

error_t tinker_exit_thread(void) TINKER_API_SUFFIX;

void tinker_debug(const char * const str) TINKER_API_SUFFIX;

#endif /* TINKER_API_PROCESS_H_ */
