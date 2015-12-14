/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifdef 0
#include "locks/lock.h"
#include "console/print_out.h"

// https://www-01.ibm.com/support/knowledgecenter/ssw_aix_61/com.ibm.aix.alangref/idalangref_lwarx_lwri_instrs.htm

void lock_init(lock_t * const lock)
{
	if (lock)
	{
		*lock = 0;
	}
}

void lock1(lock_t * const lock)
{
	if (lock)
	{
		asm volatile ("lock2:");
		asm volatile ("lwarx  %r6, 0, %r3");
		asm volatile ("addi %r4, %r0, 0");
		asm volatile ("addis %r4, %r4, 0");
		asm volatile ("bne lock2");
		asm volatile ("addi %r5, %r0, 1");
		asm volatile ("addis %r5, %r5, 0");
		asm volatile ("stwcx.  %r5, 0, %r3 ");
		asm volatile ("bne lock2");
	}
}

static void unlock1(lock_t * const lock)
{
	if (lock)
	{
		asm volatile ("addi %r5, %r0, 0");
		asm volatile ("addis %r5, %r5, 0");
		asm volatile ("stwcx.  %r5, 0, %r3 ");
	}
}

void lock(lock_t * const lock)
{
#if defined (LOCK_TRACING)
	debug_print("L %x\n", lock);
#endif
	lock1(lock);
#if defined (LOCK_TRACING)
	debug_print("Ld %x\n", lock);
#endif
}

void unlock(lock_t * const lock)
{
#if defined (LOCK_TRACING)
	debug_print("U %x\n", lock);
#endif
	unlock1(lock);
#if defined (LOCK_TRACING)
	debug_print("Ud %x\n", lock);
#endif
}
#endif
