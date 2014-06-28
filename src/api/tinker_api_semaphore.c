/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "tinker_api_semaphore.h"
#include "tinker_api_kernel_interface.h"

error_t tinker_sem_create(tinker_sem_t * semaphore, uint32_t initial_count, char * name)
{
	return TINKER_API_CALL_3(
			SYSCALL_CREATE_SEMAPHORE,
			(uint32_t)semaphore,
			(uint32_t)name,
			(uint32_t) initial_count);
}

error_t tinker_sem_open(tinker_sem_t * semaphore, char * name)
{
	return TINKER_API_CALL_2(
			SYSCALL_OPEN_SEMAPHORE,
			(uint32_t)semaphore,
			(uint32_t)name);
}

error_t tinker_sem_get(tinker_sem_t semaphore)
{
	return TINKER_API_CALL_1(
			SYSCALL_GET_SEMAPHORE,
			(uint32_t)semaphore);
}


error_t tinker_sem_release(tinker_sem_t semaphore)
{
	return TINKER_API_CALL_1(
			SYSCALL_RELEASE_SEMAPHORE,
			(uint32_t)semaphore);
}

error_t tinker_sem_close(tinker_sem_t semaphore)
{
	return TINKER_API_CALL_1(
			SYSCALL_CLOSE_SEMAPHORE,
			(uint32_t)semaphore);
}
