/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

/**
 * The length of a Powerpc General Purpose Register
 */
GPR_LENGTH = 4

/**
 * The size of the stack frame for the IVT data.
 * This is the data we wish to store.
 */
.ifdef KERNEL_POWERPC_FPS
FRAME_SIZE = 160 + (32 * 8)
.else
FRAME_SIZE = 160
.endif

/**
 * The size of the link area of the stack frame.
 * Used to navigate frames.
 */
LINK_AREA = 8

/**
 * The total size of the stack frame we need to store
 */
IVT_FRAME_SIZE = (LINK_AREA + FRAME_SIZE + LINK_AREA)

/* GPR0 is the first register at 8 */
GPR0_OFFSET = 8 + LINK_AREA
/* GPR2-31 are next */
GPR2_OFFSET = GPR0_OFFSET + GPR_LENGTH
/* The SPRs come after that */
GPR2_SIZE = (30 * 4)
SPR_SRR0_OFFSET = GPR2_OFFSET + GPR2_SIZE
SPR_SRR1_OFFSET = SPR_SRR0_OFFSET + GPR_LENGTH
SPR_XER_OFFSET = SPR_SRR1_OFFSET + GPR_LENGTH
SPR_CR_OFFSET = SPR_XER_OFFSET + GPR_LENGTH
SPR_CTR_OFFSET = SPR_CR_OFFSET + GPR_LENGTH
.ifdef KERNEL_POWERPC_FPS
FPR_SIZE = (32 * 8)
FPR_OFFSET = SPR_CTR_OFFSET + GPR_LENGTH
SPR_LR_OFFSET = FPR_OFFSET + FPR_SIZE
.else
SPR_LR_OFFSET = SPR_CTR_OFFSET + GPR_LENGTH
.endif

.macro backup_gprs
    /* backup register 0 */
    stw     %r0,    GPR0_OFFSET(%sp)
    /* store all the other GPRs */
    stmw    %r2,    GPR2_OFFSET(%sp)
.endm

.macro restore_gprs
    /* restore R2-30 */
    lmw     %r2,    GPR2_OFFSET(%sp)
    /* restore R0 */
    lwz     %r0,    GPR0_OFFSET(%sp)
.endm

.macro backup_sprs
    mflr   %r30
    mfctr  %r29
    mfcr   %r28
    mfxer  %r27
    mfsrr1 %r26
    mfsrr0 %r25
    stw     %r25,    SPR_SRR0_OFFSET(%sp)
    stw     %r26,    SPR_SRR1_OFFSET(%sp)
    stw     %r27,    SPR_XER_OFFSET(%sp)
    stw     %r28,    SPR_CR_OFFSET(%sp)
    stw     %r29,    SPR_CTR_OFFSET(%sp)
    stw     %r30,    SPR_LR_OFFSET(%sp)
.endm

.macro restore_sprs
    lwz     %r25,    SPR_SRR0_OFFSET(%sp)
    lwz     %r26,    SPR_SRR1_OFFSET(%sp)
    lwz     %r27,    SPR_XER_OFFSET(%sp)
    lwz     %r28,    SPR_CR_OFFSET(%sp)
    lwz     %r29,    SPR_CTR_OFFSET(%sp)
    lwz     %r30,    SPR_LR_OFFSET(%sp)
    mtsrr0 %r25
    mtsrr1 %r26
    mtxer  %r27
    mtcr   %r28
    mtctr  %r29
    mtlr   %r30
.endm

.global ivts

.ifdef KERNEL_POWERPC_FPS
.global ivt_backup_fpr
ivt_backup_fpr:
    /* we need to re-enabled floating point because we're in
     * the exception context */
    mfmsr   %r10
    ori     %r10,    %r10,    0x2000
    mtmsr   %r10
    /* backup all the FPRs */
    stfd    %f0,    (FPR_OFFSET + 0)(%sp)
    stfd    %f1,    (FPR_OFFSET + 8)(%sp)
    stfd    %f2,    (FPR_OFFSET + 16)(%sp)
    stfd    %f3,    (FPR_OFFSET + 24)(%sp)
    stfd    %f4,    (FPR_OFFSET + 32)(%sp)
    stfd    %f5,    (FPR_OFFSET + 40)(%sp)
    stfd    %f6,    (FPR_OFFSET + 48)(%sp)
    stfd    %f7,    (FPR_OFFSET + 56)(%sp)
    stfd    %f8,    (FPR_OFFSET + 64)(%sp)
    stfd    %f9,    (FPR_OFFSET + 72)(%sp)
    stfd    %f10,    (FPR_OFFSET + 80)(%sp)
    stfd    %f11,    (FPR_OFFSET + 88)(%sp)
    stfd    %f12,    (FPR_OFFSET + 96)(%sp)
    stfd    %f13,    (FPR_OFFSET + 104)(%sp)
    stfd    %f14,    (FPR_OFFSET + 112)(%sp)
    stfd    %f15,    (FPR_OFFSET + 120)(%sp)
    stfd    %f16,    (FPR_OFFSET + 128)(%sp)
    stfd    %f17,    (FPR_OFFSET + 136)(%sp)
    stfd    %f18,    (FPR_OFFSET + 144)(%sp)
    stfd    %f19,    (FPR_OFFSET + 152)(%sp)
    stfd    %f20,    (FPR_OFFSET + 160)(%sp)
    stfd    %f21,    (FPR_OFFSET + 168)(%sp)
    stfd    %f22,    (FPR_OFFSET + 176)(%sp)
    stfd    %f23,    (FPR_OFFSET + 184)(%sp)
    stfd    %f24,    (FPR_OFFSET + 192)(%sp)
    stfd    %f25,    (FPR_OFFSET + 200)(%sp)
    stfd    %f26,    (FPR_OFFSET + 208)(%sp)
    stfd    %f27,    (FPR_OFFSET + 216)(%sp)
    stfd    %f28,    (FPR_OFFSET + 224)(%sp)
    stfd    %f29,    (FPR_OFFSET + 232)(%sp)
    stfd    %f30,    (FPR_OFFSET + 240)(%sp)
    stfd    %f31,    (FPR_OFFSET + 248)(%sp)
    blr

.global ivt_restore_fpr
ivt_restore_fpr:
    /* make sure FP is enabled again if it wasnt before */
    mfmsr   %r10
    ori     %r10,   %r10,    0x2000
    mtmsr   %r10
    /* restore all the FP registers */
    lfd    %f0,    (FPR_OFFSET + 0)(%sp)
    lfd    %f1,    (FPR_OFFSET + 8)(%sp)
    lfd    %f2,    (FPR_OFFSET + 16)(%sp)
    lfd    %f3,    (FPR_OFFSET + 24)(%sp)
    lfd    %f4,    (FPR_OFFSET + 32)(%sp)
    lfd    %f5,    (FPR_OFFSET + 40)(%sp)
    lfd    %f6,    (FPR_OFFSET + 48)(%sp)
    lfd    %f7,    (FPR_OFFSET + 56)(%sp)
    lfd    %f8,    (FPR_OFFSET + 64)(%sp)
    lfd    %f9,    (FPR_OFFSET + 72)(%sp)
    lfd    %f10,    (FPR_OFFSET + 80)(%sp)
    lfd    %f11,    (FPR_OFFSET + 88)(%sp)
    lfd    %f12,    (FPR_OFFSET + 96)(%sp)
    lfd    %f13,    (FPR_OFFSET + 104)(%sp)
    lfd    %f14,    (FPR_OFFSET + 112)(%sp)
    lfd    %f15,    (FPR_OFFSET + 120)(%sp)
    lfd    %f16,    (FPR_OFFSET + 128)(%sp)
    lfd    %f17,    (FPR_OFFSET + 136)(%sp)
    lfd    %f18,    (FPR_OFFSET + 144)(%sp)
    lfd    %f19,    (FPR_OFFSET + 152)(%sp)
    lfd    %f20,    (FPR_OFFSET + 160)(%sp)
    lfd    %f21,    (FPR_OFFSET + 168)(%sp)
    lfd    %f22,    (FPR_OFFSET + 176)(%sp)
    lfd    %f23,    (FPR_OFFSET + 184)(%sp)
    lfd    %f24,    (FPR_OFFSET + 192)(%sp)
    lfd    %f25,    (FPR_OFFSET + 200)(%sp)
    lfd    %f26,    (FPR_OFFSET + 208)(%sp)
    lfd    %f27,    (FPR_OFFSET + 216)(%sp)
    lfd    %f28,    (FPR_OFFSET + 224)(%sp)
    lfd    %f29,    (FPR_OFFSET + 232)(%sp)
    lfd    %f30,    (FPR_OFFSET + 240)(%sp)
    lfd    %f31,    (FPR_OFFSET + 248)(%sp)
    blr
.endif

.macro ivt_macro iv

    isync
    sync

    /* store the stack pointer away and switch to the ivt stack */
    stw     %sp,    (__ivtse - IVT_FRAME_SIZE)(%r0)
    li      %sp,    __ivtse@l - IVT_FRAME_SIZE

    /* Backup the GPRs */
    backup_gprs

    /* Backup the Link register, counter, condition, exception cause */
    backup_sprs
.ifdef KERNEL_POWERPC_FPS
    bla  ivt_backup_fpr
.endif

    /* Copy over the stack pointer */
    mr     %r31,   %sp

    /* Pass the stack pointer as a parameter to the ISR */
    mr      %r4,    %r31

    /* Pass the Vector ID*/
    li     %r3,    \iv

    /* Call the ISR Handler to determine what to do */
    bla     ppc_isr_handler

.ifdef KERNEL_POWERPC_FPS
    bla  ivt_restore_fpr
.endif

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
.global ivt_system_reset_interrupt
ivt_system_reset_interrupt:

    ivt_macro iv=1

.align 1,0xFF
/* 0x200 - Machine Check */
.global ivt_machine_check_interrupt
ivt_machine_check_interrupt:

    ivt_macro iv=2

.align 1,0xFF
/* 0x300 - Data Storage Interrupt */
.global ivt_data_storage_interrupt
ivt_data_storage_interrupt:

    ivt_macro iv=3

.align 1,0xFF
/* 0x400 - Instruction Storage Interrupt */
.global ivt_inst_storage_interrupt
ivt_inst_storage_interrupt:

    ivt_macro iv=4

.align 1,0xFF
/* 0x500 - External Interrupt */
.global ivt_external_interrupt
ivt_external_interrupt:

    ivt_macro iv=5

.align 1,0xFF
/* 0x600 - Alignment Interrupt */
.global ivt_alignment_interrupt
ivt_alignment_interrupt:

    ivt_macro iv=6

.align 1,0xFF
/* 0x700 - Program Interrupt */
.global ivt_program_interrupt
ivt_program_interrupt:

    ivt_macro iv=7

.align 1,0xFF
/* 0x800 - Floating Point Unavailable Interrupt */
.global ivt_fp_unavailable
ivt_fp_unavailable:

    ivt_macro iv=8

.align 1,0xFF
/* 0x900 - Decrementer Interrupt */
.global ivt_decrementer_interrupt
ivt_decrementer_interrupt:

    ivt_macro iv=9

.align 1,0xFF
/* 0xC00 - System Call Interrupt */
.global ivt_syscall_interrupt
ivt_syscall_interrupt:

    ivt_macro iv=12

.align 1,0xFF
/* 0xD00 - Trace Interrupt */
.global ivt_trace_interrupt
ivt_trace_interrupt:

    ivt_macro iv=13

.align 1, 0xFF
/* 0xE00 - Floating Point Assist Interrupt */
.global ivt_fp_assist_interrupt
ivt_fp_assist_interrupt:

    ivt_macro iv=14
