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

#include "sos3_api_memory.h"
#include "sos3_api_kernel_interface.h"

void *	sos2_memory_malloc(uint32_t size)
{
	return (void*)SOS2_API_CALL_1(syscall_malloc, size);
}

void	sos2_memory_free(void * ptr)
{
	SOS2_API_CALL_1(syscall_mfree, (const uint32_t)ptr);
}
