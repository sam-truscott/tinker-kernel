/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
.global ppc_set_msr
ppc_set_msr:
    mtmsr %r3
    blr

.global ppc_get_msr
ppc_get_msr:
    mfmsr %r3
    blr

.global ppc_get_srr0
ppc_get_srr0:
    mfsrr0 %r3
    blr

.global ppc_get_srr1
ppc_get_srr1:
    mfsrr1 %r3
    blr

.global ppc_get_tbrs
ppc_get_tbrs:
    loop:
    mftbu   %r10
    mftbl   %r11
    mftbu   %r12
    cmp     cr0, %r10, %r12
    bne     cr0, loop
    stw     %r11, 0(%r3)
    stw     %r10, 0(%r4)
    blr

.global ppc_set_tbr
ppc_set_tbr:
    mttbu %r3
    mttbl %r4
    blr

.global ppc_set_decrementer
ppc_set_decrementer:
    mtdec   %r3
    blr

.global tgt_get_stack_pointer
tgt_get_stack_pointer:
    mr  %r3, %r1
    blr

     .global tgt_enable_external_interrupts
 tgt_enable_external_interrupts:
    mfmsr   %r10
    oris    %r10,    %r10,    0x8
    mtmsr   %r10
    blr

.global tgt_disable_external_interrupts
tgt_disable_external_interrupts:
    mfmsr   %r10
    lis     %r11,    0x8
    not     %r11,    %r11
    and     %r10,    %r10,    %r11
    mtmsr   %r10
    blr

.global ppc_set_instruction_breakpoint
ppc_set_instruction_breakpoint:
    mtspr 1010, %r3
    blr

.global ppc_set_data_breakpoint
ppc_set_data_breakpoint:
    mtspr 1013, %r3
    blr

.global ppc_get_dsisr
ppc_get_dsisr:
    mfdsisr %r3
    blr
