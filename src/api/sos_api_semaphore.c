/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "sos_api_semaphore.h"
#include "sos_api_kernel_interface.h"

error_t sos_sem_create(sos_sem_t * semaphore, uint32_t initial_count, char * name)
{
	return TINKER_API_CALL_3(
			SYSCALL_CREATE_SEMAPHORE,
			(uint32_t)semaphore,
			(uint32_t)name,
			(uint32_t) initial_count);
}

error_t sos_sem_open(sos_sem_t * semaphore, char * name)
{
	return TINKER_API_CALL_2(
			SYSCALL_OPEN_SEMAPHORE,
			(uint32_t)semaphore,
			(uint32_t)name);
}

error_t sos_sem_get(sos_sem_t semaphore)
{
	return TINKER_API_CALL_1(
			SYSCALL_GET_SEMAPHORE,
			(uint32_t)semaphore);
}


error_t sos_sem_release(sos_sem_t semaphore)
{
	return TINKER_API_CALL_1(
			SYSCALL_RELEASE_SEMAPHORE,
			(uint32_t)semaphore);
}

error_t sos_sem_close(sos_sem_t semaphore)
{
	return TINKER_API_CALL_1(
			SYSCALL_CLOSE_SEMAPHORE,
			(uint32_t)semaphore);
}
