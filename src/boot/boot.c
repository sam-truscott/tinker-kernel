/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "boot.h"
#include "kernel/kernel_main.h"

extern void kernel_boot(void)
{
	/*
	 * Kick off the kernel
	 */
	kernel_main();
}
