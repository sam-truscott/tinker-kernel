/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "sos_api_semaphore.h"
#include "sos_api_kernel_interface.h"

error_t sos_sem_create(sos_sem * semaphore, uint32_t initial_count)
{
	return SOS_API_CALL_2(
			syscall_create_semaphore,
			(uint32_t)semaphore,
			(uint32_t) initial_count);
}


error_t sos_sem_get(sos_sem semaphore)
{
	return SOS_API_CALL_1(
			syscall_get_semaphore,
			(uint32_t)semaphore);
}


error_t sos_sem_release(sos_sem semaphore)
{
	return SOS_API_CALL_1(
			syscall_release_semaphore,
			(uint32_t)semaphore);
}
