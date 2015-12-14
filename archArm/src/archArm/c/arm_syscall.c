/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "tinker_api_kernel_interface.h"
#include "kernel_assert.h"

#pragma GCC optimize ("-O0")
uint32_t TINKER_API_CALL_7(
		const syscall_function_t api,
		uint32_t param_1,
		uint32_t param_2,
		uint32_t param_3,
		uint32_t param_4,
		uint32_t param_5,
		uint32_t param_6,
		uint32_t param_7)
{
	kernel_assert("tinker: syscall error\n", api<MAX_SYSCALL);
	register long _r0 __asm__("r0")=(long)(api);
	register long _r7 __asm__("r7")=(long)(param_7);
	register long _r6 __asm__("r6")=(long)(param_6);
	register long _r5 __asm__("r5")=(long)(param_5);
	register long _r4 __asm__("r4")=(long)(param_4);
	register long _r3 __asm__("r3")=(long)(param_3);
	register long _r2 __asm__("r2")=(long)(param_2);
	register long _r1 __asm__("r1")=(long)(param_1);
	__asm__ __volatile__(
			"svc 0"
			: "=r"(_r0)
			: "r"(_r0), "r"(_r1),
			"r"(_r2), "r"(_r3), "r"(_r4), "r"(_r5),
			"r"(_r6), "r"(_r7)
			: "memory");
	return (long) _r0;
}
