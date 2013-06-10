/*************************************************************************
 *
 * SOS 3 Source Code
 * __________________
 *
 *  [2012] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

.global __ppc_set_msr
__ppc_set_msr:
    mtmsr %r3
    blr

.global __ppc_get_msr
__ppc_get_msr:
    mfmsr %r3
    blr

.global __ppc_get_srr0
__ppc_get_srr0:
    mfsrr0 %r3
    blr

.global __ppc_get_srr1
__ppc_get_srr1:
    mfsrr1 %r3
    blr

.global __ppc_get_tbrs
__ppc_get_tbrs:
    loop:
    mftbu   %r10
    mftbl   %r11
    mftbu   %r12
    cmp     cr0, %r10, %r12
    bne     cr0, loop
    stw     %r11, 0(%r3)
    stw     %r10, 0(%r4)
    blr

.global __ppc_set_tbr
__ppc_set_tbr:
    mttbu %r3
    mttbl %r4
    blr

.global __ppc_set_decrementer
__ppc_set_decrementer:
    mtdec   %r3
    blr

.global __tgt_get_stack_pointer
__tgt_get_stack_pointer:
    mr  %r3, %r1
    blr

     .global __tgt_enable_external_interrupts
 __tgt_enable_external_interrupts:
    mfmsr   %r10
    oris    %r10,    %r10,    0x8
    mtmsr   %r10
    blr

.global __tgt_disable_external_interrupts
__tgt_disable_external_interrupts:
    mfmsr   %r10
    lis     %r11,    0x8
    not     %r11,    %r11
    and     %r10,    %r10,    %r11
    mtmsr   %r10
    blr

.global __ppc_set_instruction_breakpoint
__ppc_set_instruction_breakpoint:
    mtspr 1010, %r3
    blr

.global __ppc_set_data_breakpoint
__ppc_set_data_breakpoint:
    mtspr 1013, %r3
    blr

.global __ppc_get_dsisr
__ppc_get_dsisr:
    mfdsisr %r3
    blr
