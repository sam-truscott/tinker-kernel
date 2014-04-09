/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "sos_api_shm.h"
#include "sos_api_kernel_interface.h"

error_t sos_shm_create(sos_shm_t * shm, char * name, uint32_t size, void** addr)
{
	return TINKER_API_CALL_4(
			SYSCALL_CREATE_SHM,
			(uint32_t)shm,
			(uint32_t)name,
			(uint32_t)size,
			(uint32_t)addr);
}

error_t sos_shm_open(sos_shm_t * shm, char * name, uint32_t size, void** addr)
{
	return TINKER_API_CALL_4(
			SYSCALL_OPEN_SHM,
			(uint32_t)shm,
			(uint32_t)name,
			(uint32_t)size,
			(uint32_t)addr);
}

error_t sos_shm_destroy(sos_shm_t shm)
{
	return TINKER_API_CALL_1(
			SYSCALL_DESTROY_SHM,
			(uint32_t)shm);
}
