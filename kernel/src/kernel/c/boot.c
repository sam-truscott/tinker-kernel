/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "boot.h"
#include "kernel_main.h"
#include "board_support.h"

typedef void (kmainptr)(void);

extern void kernel_boot(void)
{
	bsp_initialise();

	kernel_main();
	/*
	 * Kick off the kernel
	 */
	kmainptr * const kmain = (kmainptr*)(((mem_t)kernel_main) + KERNEL_ADDRESS_SPACE);
	if (kmain)
	{
		kmain();
	}
}
