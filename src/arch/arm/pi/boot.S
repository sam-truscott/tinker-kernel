/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

 .section .entry, "ax"

 # setting up the Multiboot header - see GRUB docs for details
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

/*
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
*/

.global kernel_start
kernel_start:
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
