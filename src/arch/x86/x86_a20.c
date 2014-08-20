/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "x86_registers.h"
#include "kernel/console/print_out.h"

#define MAX_ATTEMPTS 255
#define TEST_ADDRESS (4*0x80)

static bool_t x86_a20_check(const uint8_t iterations)
{
	x86_set_fs(0x0000);
	x86_set_gs(0xFFFF);

	uint32_t tmp, ctr;
	uint8_t iteration = iterations;
	bool_t ok = false;

	printp_out("A20 Check\n", iteration);
	tmp = ctr = in_u32(TEST_ADDRESS);
	while(iteration)
	{
		printp_out("A20 Check, attempt %d\n", iteration);
		out_u32(TEST_ADDRESS, ctr++);
		asm volatile("outb %%al,%0" : : "dN" (0x80));
		if ((ok=in_u32(TEST_ADDRESS+0x10) ^ ctr)==true)
		{
			break;
		}
		iteration--;
	}
	out_u32(tmp, TEST_ADDRESS);
	return ok;
}

bool_t x86_enable_a20(void)
{
	uint8_t iteration = MAX_ATTEMPTS;
	bool_t ok = false;
	while (iteration)
	{
		printp_out("A20 Enable, attempt %d\n", iteration);
		if ((ok=x86_a20_check(32)))
		{
			break;
		}
		iteration--;
	}
}
