/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_PROCESS_H_
#define SOS_API_PROCESS_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"
#include "sos_api_kernel_interface.h"

/**
 * The entry point for a new thread
 */
typedef void (thread_entry_point)(void);

/**
 * The type in user space for a process
 */
typedef object_number_t sos_process_t;

/**
 * The type in user space for a thread
 */
typedef object_number_t sos_thread_t;

typedef struct sos_meminfo {
	uint32_t heap_size;
	uint32_t stack_size;
	uint32_t text_start;
	uint32_t text_size;
	uint32_t data_start;
	uint32_t data_size;
} sos_meminfo_t;

error_t sos_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		const sos_meminfo_t * const meminfo,
		uint32_t flags,
		sos_process_t * process) SOS_API_SUFFIX;

error_t sos_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		sos_thread_t * thread) SOS_API_SUFFIX;

error_t sos_get_thread_object(sos_thread_t * thread) SOS_API_SUFFIX ;

error_t	sos_get_thread_priority(
		sos_thread_t thread,
		uint8_t * priority) SOS_API_SUFFIX;

error_t sos_exit_thread(void) SOS_API_SUFFIX;

void sos_debug(const char * const str) SOS_API_SUFFIX;

#endif /* SOS_API_PROCESS_H_ */
