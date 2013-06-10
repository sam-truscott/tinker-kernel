/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
extern unsigned long long __udivmoddi4(
		unsigned long long num,
		unsigned long long den,
		unsigned long long *rem_p)
{
	unsigned long long quot = 0, qbit = 1;

	if (den == 0) {
		/* __divide_error(); */
		return 0; /* If trap returns... */
	}

	/* Left-justify denominator and count shift */
	while ((long long) den >= 0) {
		den <<= 1;
		qbit <<= 1;
	}

	while (qbit) {
		if (den <= num) {
			num -= den;
			quot += qbit;
		}
		den >>= 1;
		qbit >>= 1;
	}

	if (rem_p)
		*rem_p = num;

	return quot;
}
