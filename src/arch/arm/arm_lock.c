/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "kernel/locks/lock.h"

// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dht0008a/CJHBGBBJ.html

void lock_init(lock_t * const lock)
{
	if (lock)
	{
		*lock = 0;
	}
}

void __attribute__((naked)) lock(lock_t * const lock)
{
	if (lock)
	{
		asm volatile ("LDR r2, =1");
		asm volatile ("SWP r1, r2, [r0]");
		asm volatile ("CMP r1, r2");
		asm volatile ("BEQ lock");
		asm volatile ("BX  lr");
	}
	else
	{
		asm volatile ("BX  lr");
	}
}

void __attribute__((naked)) unlock(lock_t * const lock)
{
	if (lock)
	{
		(void)lock;
		asm volatile ("LDR r1, =0");
		asm volatile ("STR r1, [r0]");
		asm volatile ("BX  lr");
	}
	else
	{
		asm volatile ("BX  lr");
	}
}
