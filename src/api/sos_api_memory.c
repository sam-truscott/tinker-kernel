/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "sos_api_memory.h"
#include "sos_api_kernel_interface.h"

void *	sos_memory_malloc(uint32_t size)
{
	return (void*)SOS_API_CALL_1(syscall_malloc, size);
}

void	sos_memory_free(void * ptr)
{
	SOS_API_CALL_1(syscall_mfree, (const uint32_t)ptr);
}
