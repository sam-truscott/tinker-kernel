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
	msr cpsr, #0xD1			/* enter fiq mode & disable interrupts */
	ldr sp, =__ivtse		/* setup fiq stack */
	msr cpsr, #0xD2			/* enter irq mode & disable interrupts */
	ldr sp, =__ivtse		/* setup irq stack */
	msr cpsr, #0xDB			/* enter und mode & disable interrupts */
	ldr sp, =__ivtse		/* setup und stack */
	msr cpsr, #0xD7			/* enter abt mode & disable interrupts */
	ldr sp, =__ivtse		/* setup abt stack */
	msr cpsr, #0xD3 		/* enter svc mode & disable interrupts */
	ldr sp, =__ivtse		/* setup svc stack */
	msr cpsr, #0xDF			/* enter sys mode & disable interrupts */
	ldr sp, =__bootse		/* setup sys stack */

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
	bl	kernel_boot

halt:
	b	halt
