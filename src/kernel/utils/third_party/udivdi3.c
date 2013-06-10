/*
 * udivdi3.c
 *
 *  Created on: 20 Jul 2011
 *      Author: other
 */

extern unsigned long long __udivmoddi4(
		unsigned long long num,
		unsigned long long den,
		unsigned long long *rem_p);

extern unsigned long long __udivdi3(unsigned long long num, unsigned long long den)
{
	  return __udivmoddi4(num, den, 0);
}

/*
 * This code is from:
 * http://code.coreboot.org/p/openbios/source/tree/HEAD/tags/openbios-1.0/libgcc/__udivdi3.c
 * http://code.coreboot.org/p/openbios/source/tree/HEAD/tags/openbios-1.0/libgcc/__udivmoddi4.c
 *
 * It's to mimic the __udivdi3 function present in libgcc that we don't include.
 */
