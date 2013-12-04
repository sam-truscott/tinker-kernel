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

error_t sos_sem_create(sos_sem_t * semaphore, uint32_t initial_count, char * name)
{
	return SOS_API_CALL_3(
			syscall_create_semaphore,
			(uint32_t)semaphore,
			(uint32_t)name,
			(uint32_t) initial_count);
}

error_t sos_sem_open(sos_sem_t * semaphore, char * name)
{
	return SOS_API_CALL_2(
			syscall_open_semaphore,
			(uint32_t)semaphore,
			(uint32_t)name);
}

error_t sos_sem_get(sos_sem_t semaphore)
{
	return SOS_API_CALL_1(
			syscall_get_semaphore,
			(uint32_t)semaphore);
}


error_t sos_sem_release(sos_sem_t semaphore)
{
	return SOS_API_CALL_1(
			syscall_release_semaphore,
			(uint32_t)semaphore);
}
