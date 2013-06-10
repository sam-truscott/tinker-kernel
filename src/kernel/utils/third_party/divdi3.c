/*
 * divdi3.c
 *
 *  Created on: 23 Jul 2011
 *      Author: sam
 */

extern unsigned long long __udivmoddi4(
		unsigned long long num,
		unsigned long long den,
		unsigned long long *rem_p);

signed long long __divdi3(signed long long num, signed long long den)
{
	int minus = 0;
	signed long long v;

	if (num < 0)
	{
		num = -num;
		minus = 1;
	}
	if (den < 0)
	{
		den = -den;
		minus ^= 1;
	}

	v = __udivmoddi4(num, den, 0);
	if (minus)
		v = -v;

	return v;
}
