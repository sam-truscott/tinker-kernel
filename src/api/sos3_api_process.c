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

#include "sos3_api_process.h"
#include "sos3_api.h"
#include "sos3_api_kernel_interface.h"

/**
 * Create a new Process
 * @param image_name The name of the process
 * @param initial_thread_name The name of the initial thread/task
 * @param entry The entry point of the initial thread/task
 * @param priority The priority of the initial thread/task
 * @param heap The amount of heap for the process. This will be taken from the
 * parent process. If the parent process does not have enough spare heap for
 * this new process to be created then it will fail.
 * @param stack The amount of stack for th einitial threa/task.
 * @param process A pointer to the created Process.
 * @param thread A pointer to the created Thread.
 * @return
 */
error_t sos2_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t heap,
		uint32_t stack,
		uint32_t flags,
		sos2_process * process)
{
	return SOS2_API_CALL_7(
			syscall_create_process,
			(uint32_t)image_name,
			(uint32_t)entry,
			(uint32_t)priority,
			stack,
			heap,
			flags,
			(uint32_t)process);
}

error_t sos2_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		sos2_thread * thread)
{
	return SOS2_API_CALL_6(
			syscall_create_thread,
			(uint32_t)thread_name,
			(uint32_t)entry,
			(uint32_t)priority,
			stack,
			flags,
			(uint32_t)thread);
}

error_t sos2_get_thread_object(sos2_thread * thread)
{
	return SOS2_API_CALL_1(
			syscall_thread_object,
			(uint32_t)thread);
}

error_t	sos2_get_thread_priority(
		sos2_thread thread,
		uint8_t * priority)
{
	return SOS2_API_CALL_2(
			syscall_thread_priority,
			(uint32_t)thread,
			(uint32_t)priority);
}

error_t sos2_exit_thread(void)
{
	return SOS2_API_CALL_0(syscall_exit_thread);
}
