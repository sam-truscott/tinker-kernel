/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "tinker_api_shm.h"
#include "tinker_api_kernel_interface.h"

return_t tinker_shm_create(tinker_shm_t * shm, char * name, uint32_t size, void** addr)
{
	return TINKER_API_CALL_4(
			SYSCALL_CREATE_SHM,
			(uint32_t)shm,
			(uint32_t)name,
			(uint32_t)size,
			(uint32_t)addr);
}

return_t tinker_shm_open(tinker_shm_t * shm, char * name, uint32_t size, void** addr)
{
	return TINKER_API_CALL_4(
			SYSCALL_OPEN_SHM,
			(uint32_t)shm,
			(uint32_t)name,
			(uint32_t)size,
			(uint32_t)addr);
}

return_t tinker_shm_destroy(tinker_shm_t shm)
{
	return TINKER_API_CALL_1(
			SYSCALL_DESTROY_SHM,
			(uint32_t)shm);
}
