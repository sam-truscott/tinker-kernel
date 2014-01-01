/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

/**
 * The length of a Powerpc General Purpose Register
 */
__GPR_LENGTH = 4

/**
 * The size of the stack frame for the IVT data.
 * This is the data we wish to store.
 */
__FRAME_SIZE = 160 + (32 * 8)

/**
 * The size of the link area of the stack frame.
 * Used to navigate frames.
 */
__LINK_AREA = 8

/**
 * The total size of the stack frame we need to store
 */
__IVT_FRAME_SIZE = (__LINK_AREA + __FRAME_SIZE + __LINK_AREA)

/* GPR0 is the first register at 8 */
__GPR0_OFFSET = 8 + __LINK_AREA
/* GPR2-31 are next */
__GPR2_OFFSET = __GPR0_OFFSET + __GPR_LENGTH
/* The SPRs come after that */
__GPR2_SIZE = (30 * 4)
__SPR_SRR0_OFFSET = __GPR2_OFFSET + __GPR2_SIZE
__SPR_SRR1_OFFSET = __SPR_SRR0_OFFSET + __GPR_LENGTH
__SPR_XER_OFFSET = __SPR_SRR1_OFFSET + __GPR_LENGTH
__SPR_CR_OFFSET = __SPR_XER_OFFSET + __GPR_LENGTH
__SPR_CTR_OFFSET = __SPR_CR_OFFSET + __GPR_LENGTH
__FPR_SIZE = (32 * 8)
__FPR_OFFSET = __SPR_CTR_OFFSET + __GPR_LENGTH
__SPR_LR_OFFSET = __FPR_OFFSET + __FPR_SIZE

.macro backup_gprs
    /* backup register 0 */
    stw     %r0,    __GPR0_OFFSET(%sp)
    /* store all the other GPRs */
    stmw    %r2,    __GPR2_OFFSET(%sp)
.endm

.macro restore_gprs
    /* restore R2-30 */
    lmw     %r2,    __GPR2_OFFSET(%sp)
    /* restore R0 */
    lwz     %r0,    __GPR0_OFFSET(%sp)
.endm

.macro backup_sprs
    mflr   %r30
    mfctr  %r29
    mfcr   %r28
    mfxer  %r27
    mfsrr1 %r26
    mfsrr0 %r25
    stw     %r25,    __SPR_SRR0_OFFSET(%sp)
    stw     %r26,    __SPR_SRR1_OFFSET(%sp)
    stw     %r27,    __SPR_XER_OFFSET(%sp)
    stw     %r28,    __SPR_CR_OFFSET(%sp)
    stw     %r29,    __SPR_CTR_OFFSET(%sp)
    stw     %r30,    __SPR_LR_OFFSET(%sp)
.endm

.macro restore_sprs
    lwz     %r25,    __SPR_SRR0_OFFSET(%sp)
    lwz     %r26,    __SPR_SRR1_OFFSET(%sp)
    lwz     %r27,    __SPR_XER_OFFSET(%sp)
    lwz     %r28,    __SPR_CR_OFFSET(%sp)
    lwz     %r29,    __SPR_CTR_OFFSET(%sp)
    lwz     %r30,    __SPR_LR_OFFSET(%sp)
    mtsrr0 %r25
    mtsrr1 %r26
    mtxer  %r27
    mtcr   %r28
    mtctr  %r29
    mtlr   %r30
.endm

.global __ivts

.global __ivt_backup_fpr
__ivt_backup_fpr:
    /* get the SRR1 - containing the FP flag - and work
     * out if floating point is enabled. if so backup all
     * the registers */
    mfsrr1  %r10
    andi.   %r10,   %r10,    0x2000
    cmpi    0,      0,      %r10,      0x2000
    beq     0,      __fp_backup_disabled /* FIXME: Shouldn't this be BNEQ 0x2000 or BEQ 0x0? */
    /* we need to re-enabled floating point because we're n
     * the exception context */
    li      %r5,    1
    mfmsr   %r10
    ori     %r10,    %r10,    0x2000
    mtmsr   %r10
    /* backup all the FPRs */
    stfd    %f0,    (__FPR_OFFSET + 0)(%sp)
    stfd    %f1,    (__FPR_OFFSET + 8)(%sp)
    stfd    %f2,    (__FPR_OFFSET + 16)(%sp)
    stfd    %f3,    (__FPR_OFFSET + 24)(%sp)
    stfd    %f4,    (__FPR_OFFSET + 32)(%sp)
    stfd    %f5,    (__FPR_OFFSET + 40)(%sp)
    stfd    %f6,    (__FPR_OFFSET + 48)(%sp)
    stfd    %f7,    (__FPR_OFFSET + 56)(%sp)
    stfd    %f8,    (__FPR_OFFSET + 64)(%sp)
    stfd    %f9,    (__FPR_OFFSET + 72)(%sp)
    stfd    %f10,    (__FPR_OFFSET + 80)(%sp)
    stfd    %f11,    (__FPR_OFFSET + 88)(%sp)
    stfd    %f12,    (__FPR_OFFSET + 96)(%sp)
    stfd    %f13,    (__FPR_OFFSET + 104)(%sp)
    stfd    %f14,    (__FPR_OFFSET + 112)(%sp)
    stfd    %f15,    (__FPR_OFFSET + 120)(%sp)
    stfd    %f16,    (__FPR_OFFSET + 128)(%sp)
    stfd    %f17,    (__FPR_OFFSET + 136)(%sp)
    stfd    %f18,    (__FPR_OFFSET + 144)(%sp)
    stfd    %f19,    (__FPR_OFFSET + 152)(%sp)
    stfd    %f20,    (__FPR_OFFSET + 160)(%sp)
    stfd    %f21,    (__FPR_OFFSET + 168)(%sp)
    stfd    %f22,    (__FPR_OFFSET + 176)(%sp)
    stfd    %f23,    (__FPR_OFFSET + 184)(%sp)
    stfd    %f24,    (__FPR_OFFSET + 192)(%sp)
    stfd    %f25,    (__FPR_OFFSET + 200)(%sp)
    stfd    %f26,    (__FPR_OFFSET + 208)(%sp)
    stfd    %f27,    (__FPR_OFFSET + 216)(%sp)
    stfd    %f28,    (__FPR_OFFSET + 224)(%sp)
    stfd    %f29,    (__FPR_OFFSET + 232)(%sp)
    stfd    %f30,    (__FPR_OFFSET + 240)(%sp)
    stfd    %f31,    (__FPR_OFFSET + 248)(%sp)
    blr
__fp_backup_disabled:
    li      %r5,    0
    blr

.global __ivt_restore_fpr
__ivt_restore_fpr:
    /* get the SRR1 from the stack (backed up context) and
     * check if we need to restore floating point access
     * and then copy over all the registers */
    lwz     %r10,    __SPR_SRR1_OFFSET(%sp)
    andi.   %r10,    %r10,    0x2000
    cmpi    0,       0,      %r10,      0x2000
    beq     0,      __fp_restore_disabled /* FIXME: Shouldn't this be BNEQ 0x2000 or BEQ 0x0? */
    /* make sure FP is enabled again if it wasnt before */
    mfmsr   %r10
    ori     %r10,   %r10,    0x2000
    mtmsr   %r10
    /* restore all the FP registers */
    lfd    %f0,    (__FPR_OFFSET + 0)(%sp)
    lfd    %f1,    (__FPR_OFFSET + 8)(%sp)
    lfd    %f2,    (__FPR_OFFSET + 16)(%sp)
    lfd    %f3,    (__FPR_OFFSET + 24)(%sp)
    lfd    %f4,    (__FPR_OFFSET + 32)(%sp)
    lfd    %f5,    (__FPR_OFFSET + 40)(%sp)
    lfd    %f6,    (__FPR_OFFSET + 48)(%sp)
    lfd    %f7,    (__FPR_OFFSET + 56)(%sp)
    lfd    %f8,    (__FPR_OFFSET + 64)(%sp)
    lfd    %f9,    (__FPR_OFFSET + 72)(%sp)
    lfd    %f10,    (__FPR_OFFSET + 80)(%sp)
    lfd    %f11,    (__FPR_OFFSET + 88)(%sp)
    lfd    %f12,    (__FPR_OFFSET + 96)(%sp)
    lfd    %f13,    (__FPR_OFFSET + 104)(%sp)
    lfd    %f14,    (__FPR_OFFSET + 112)(%sp)
    lfd    %f15,    (__FPR_OFFSET + 120)(%sp)
    lfd    %f16,    (__FPR_OFFSET + 128)(%sp)
    lfd    %f17,    (__FPR_OFFSET + 136)(%sp)
    lfd    %f18,    (__FPR_OFFSET + 144)(%sp)
    lfd    %f19,    (__FPR_OFFSET + 152)(%sp)
    lfd    %f20,    (__FPR_OFFSET + 160)(%sp)
    lfd    %f21,    (__FPR_OFFSET + 168)(%sp)
    lfd    %f22,    (__FPR_OFFSET + 176)(%sp)
    lfd    %f23,    (__FPR_OFFSET + 184)(%sp)
    lfd    %f24,    (__FPR_OFFSET + 192)(%sp)
    lfd    %f25,    (__FPR_OFFSET + 200)(%sp)
    lfd    %f26,    (__FPR_OFFSET + 208)(%sp)
    lfd    %f27,    (__FPR_OFFSET + 216)(%sp)
    lfd    %f28,    (__FPR_OFFSET + 224)(%sp)
    lfd    %f29,    (__FPR_OFFSET + 232)(%sp)
    lfd    %f30,    (__FPR_OFFSET + 240)(%sp)
    lfd    %f31,    (__FPR_OFFSET + 248)(%sp)
__fp_restore_disabled:
    blr

.macro __ivt_macro iv

    isync
    sync

    /* store the stack pointer away and switch to the ivt stack */
    /* FIXME bug? LI falls over which could be because SRs are wrong */
    stw     %sp,    (__ivtse - __IVT_FRAME_SIZE)(%r0)
    li      %sp,    __ivtse@l - __IVT_FRAME_SIZE

    /* Backup the GPRs */
    backup_gprs

    /* Backup the Link register, counter, condition, exception cause */
    backup_sprs

    bla  __ivt_backup_fpr

    /* Copy over the stack pointer */
    mr     %r31,   %sp

    /* Pass the stack pointer as a parameter to the ISR */
    mr      %r4,    %r31

    /* Pass the Vector ID*/
    li     %r3,    \iv

    /* Call the ISR Handler to determine what to do */
    bla     __ppc_isr_handler

    bla  __ivt_restore_fpr

    /* Restore the special purpose registers */
    restore_sprs

    /* Restore the general purpose registers */
    restore_gprs

    /* Restore the next stack frame */
    lwz     %sp,    0(%sp)

    sync
    isync

    /* Return from the interrupt */
    rfi
.endm

.section .vector_code

/* 0x100 System Reset Interrupt */
.global __ivt_system_reset_interrupt
__ivt_system_reset_interrupt:

    __ivt_macro iv=1

.align 1,0xFF
/* 0x200 - Machine Check */
.global __ivt_machine_check_interrupt
__ivt_machine_check_interrupt:

    __ivt_macro iv=2

.align 1,0xFF
/* 0x300 - Data Storage Interrupt */
.global __ivt_data_storage_interrupt
__ivt_data_storage_interrupt:

    __ivt_macro iv=3

.align 1,0xFF
/* 0x400 - Instruction Storage Interrupt */
.global __ivt_inst_storage_interrupt
__ivt_inst_storage_interrupt:

    __ivt_macro iv=4

.align 1,0xFF
/* 0x500 - External Interrupt */
.global __ivt_external_interrupt
__ivt_external_interrupt:

    __ivt_macro iv=5

.align 1,0xFF
/* 0x600 - Alignment Interrupt */
.global __ivt_alignment_interrupt
__ivt_alignment_interrupt:

    __ivt_macro iv=6

.align 1,0xFF
/* 0x700 - Program Interrupt */
.global __ivt_program_interrupt
__ivt_program_interrupt:

    __ivt_macro iv=7

.align 1,0xFF
/* 0x800 - Floating Point Unavailable Interrupt */
.global __ivt_fp_unavailable
__ivt_fp_unavailable:

    __ivt_macro iv=8

.align 1,0xFF
/* 0x900 - Decrementer Interrupt */
.global __ivt_decrementer_interrupt
__ivt_decrementer_interrupt:

    __ivt_macro iv=9

.align 1,0xFF
/* 0xC00 - System Call Interrupt */
.global __ivt_syscall_interrupt
__ivt_syscall_interrupt:

    __ivt_macro iv=12

.align 1,0xFF
/* 0xD00 - Trace Interrupt */
.global __ivt_trace_interrupt
__ivt_trace_interrupt:

    __ivt_macro iv=13

.align 1, 0xFF
/* 0xE00 - Floating Point Assist Interrupt */
.global __ivt_fp_assist_interrupt
__ivt_fp_assist_interrupt:

    __ivt_macro iv=14
