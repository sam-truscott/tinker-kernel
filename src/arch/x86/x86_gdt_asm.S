/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

.global x86_load_gdt
x86_load_gdt:
	movl 4(%esp), %eax
	lgdt (%eax)
	ret
