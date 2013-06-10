/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef SOS2_API_PROCESS_H_
#define SOS2_API_PROCESS_H_

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
typedef object_number_t sos2_process;

/**
 * The type in user space for a thread
 */
typedef object_number_t sos2_thread;

error_t sos2_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t heap,
		uint32_t stack,
		uint32_t flags,
		sos2_process * process) SOS3_API_SUFFIX;

error_t sos2_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		sos2_thread * thread) SOS3_API_SUFFIX;

error_t sos2_get_thread_object(sos2_thread * thread) SOS3_API_SUFFIX ;

error_t	sos2_get_thread_priority(
		sos2_thread thread,
		uint8_t * priority) SOS3_API_SUFFIX;

error_t sos2_exit_thread(void) SOS3_API_SUFFIX;

#endif /* SOS2_API_PROCESS_H_ */
