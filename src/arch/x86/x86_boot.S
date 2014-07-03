/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

 # setting up the Multiboot header - see GRUB docs for details
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum required

 .section .boot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

//.section .bootstrap_stack,"aw", @nobits
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global __kernel_start
__kernel_start:
	movl $stack_top, %esp
    //cli
    call     kernel_boot
    cli
	hlt
.Lhang:
	jmp .Lhang
