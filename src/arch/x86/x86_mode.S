/*
 * x86_mode.S
 *
 *  Created on: 17 Aug 2014
 *      Author: sam
 */
.global x86_enter_protected
x86_enter_protected:
.code16
	movl %cr0, %eax
    orl  $1,   %eax
    movl %eax, %cr0

    /* Reload CS with a far jump */
    ljmpl $0x08, $1f

.code32
1:	ret
