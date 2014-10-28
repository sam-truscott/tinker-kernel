/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

 .section .entry, "ax"

.global kernel_start
kernel_start:
	mrs r3, cpsr 			/* backup cpsr */
	msr cpsr, #0x51			/* enter fiq mode*/
	ldr sp, =__ivtse		/* setup fiq stack */
	msr cpsr, #0x72			/* enter irq mode */
	ldr sp, =__ivtse		/* setup irq stack */
	msr cpsr, r3			/* restore old cpsr */
	ldr	sp, =__ivtse
	// Clear out bss.
	ldr	r4, =_bss_start
	ldr	r9, =_bss_end
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
    b       2f

1:
	// store multiple at r4.
	stmia	r4!, {r5-r8}

	// If we are still below bss_end, loop.
2:
	cmp	r4, r9
	blo	1b

	ldr	r3, =kernel_boot
	blx	r3

halt:
	wfe
	b	halt
