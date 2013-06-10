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

#include "sos3_api_semaphore.h"
#include "sos3_api_kernel_interface.h"

error_t sos2_sem_create(sos2_sem * semaphore, uint32_t initial_count)
{
	return SOS2_API_CALL_2(
			syscall_create_semaphore,
			(uint32_t)semaphore,
			(uint32_t) initial_count);
}


error_t sos2_sem_get(sos2_sem semaphore)
{
	return SOS2_API_CALL_1(
			syscall_get_semaphore,
			(uint32_t)semaphore);
}


error_t sos2_sem_release(sos2_sem semaphore)
{
	return SOS2_API_CALL_1(
			syscall_release_semaphore,
			(uint32_t)semaphore);
}
