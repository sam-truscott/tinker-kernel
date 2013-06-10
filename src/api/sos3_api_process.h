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

#include "sos3_api_types.h"
#include "sos3_api_errors.h"
#include "sos3_api_kernel_interface.h"

/**
 * The entry point for a new thread
 */
typedef void (thread_entry_point)(void);

/**
 * The type in user space for a process
 */
typedef object_number_t sos_process;

/**
 * The type in user space for a thread
 */
typedef object_number_t sos_thread;

error_t sos_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t heap,
		uint32_t stack,
		uint32_t flags,
		sos_process * process) SOS_API_SUFFIX;

error_t sos_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		sos_thread * thread) SOS_API_SUFFIX;

error_t sos_get_thread_object(sos_thread * thread) SOS_API_SUFFIX ;

error_t	sos_get_thread_priority(
		sos_thread thread,
		uint8_t * priority) SOS_API_SUFFIX;

error_t sos_exit_thread(void) SOS_API_SUFFIX;

#endif /* SOS_API_PROCESS_H_ */
