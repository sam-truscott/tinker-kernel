.section .text
.global arm_bootstrap
.type arm_bootstrap, %function

arm_bootstrap:
    // --- Set up the initial stack ---
    mov x29, x2          // fp = sp argument
    mov sp, x2           // sp = provided stack pointer
    // Optional: allocate space for local frame (if needed)
    sub sp, sp, #128     // reserve 128 bytes for frame/context

    // --- Save initial arguments on stack (optional) ---
    stp x0, x1, [sp, #16]  // store entry + exit_function
    str x2, [sp, #32]      // store original sp (optional)

    // --- Enable interrupts (DAIF) ---
    msr daifclr, #0xF       // clear Debug, SError, IRQ, FIQ masks

    // --- Call entry function ---
    mov x0, x0              // entry function pointer already in x0
    mov x1, x2              // pass exit_function as second argument
    blr x0                   // call entry function pointer

    // --- Call exit function after entry returns ---
    mov x0, x1               // exit_function pointer in x0
    blr x0                   // call exit function

    // --- Restore stack/frame ---
    add sp, x29, #0          // restore original sp
    ret
