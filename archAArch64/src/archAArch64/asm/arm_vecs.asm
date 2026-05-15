  .section .text
    .syntax unified
    .global arm_vector_reset_entry
    .global arm_vector_undefined_entry
    .global arm_vector_prefetch_abort_entry
    .global arm_vector_data_abort_entry
    .global arm_vector_reserved_entry
    .global arm_vector_irq_entry
    .global arm_vector_fiq_entry
    .global arm_vector_system_call_entry

// ---------------------- Vector Entry Macro ----------------------
.macro VECTOR_ENTRY name, handler
    .global \name
\name:
	mov x31, sp             // x31 = current SP
    str x31, [sp, #-8]!     // push SP on stack
    // Save general-purpose registers x0-x29
    stp x0,  x1,  [sp, #-16]!
    stp x2,  x3,  [sp, #-16]!
    stp x4,  x5,  [sp, #-16]!
    stp x6,  x7,  [sp, #-16]!
    stp x8,  x9,  [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!

     // --- Save LR (x30) ---
    str x30, [sp, #-8]!

    // --- Save SPSR_EL1 ---
    mrs x0, spsr_el1
    str x0, [sp, #-8]!

    // --- Adjust LR for exception return if needed ---
    sub x30, x30, #4

    // Call C handler
    bl \handler

   // --- Restore SPSR_EL1 ---
    ldr x0, [sp], #8
    msr spsr_el1, x0

    // --- Restore LR ---
    ldr x30, [sp], #8

    // --- Restore general-purpose registers ---
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8,  x9,  [sp], #16
    ldp x6,  x7,  [sp], #16
    ldp x4,  x5,  [sp], #16
    ldp x2,  x3,  [sp], #16
    ldp x0,  x1,  [sp], #16

    // --- Restore SP of interrupted context ---
    ldr sp, [sp], #8

    // --- Return from exception ---
    eret
.endm

// ---------------------- Generate All Entries ----------------------
VECTOR_ENTRY arm_vector_reset_entry, arm_vector_reset_handler
VECTOR_ENTRY arm_vector_undefined_entry, arm_vector_undefined_handler
VECTOR_ENTRY arm_vector_prefetch_abort_entry, arm_vector_prefetch_abort_handler
VECTOR_ENTRY arm_vector_data_abort_entry, arm_vector_data_abort_handler
VECTOR_ENTRY arm_vector_reserved_entry, arm_vector_reserved_handler
VECTOR_ENTRY arm_vector_irq_entry, arm_vector_irq_handler
VECTOR_ENTRY arm_vector_fiq_entry, arm_vector_fiq_handler
VECTOR_ENTRY arm_vector_system_call_entry, arm_vector_system_call_handler
