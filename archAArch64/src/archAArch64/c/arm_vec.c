/*
 * TINKER Source Code - AArch64 Vector Table (Safe Naked Entry)
 *
 *  [2009] - [2026] Samuel Steven Truscott (ported)
 */

#include "arm_vec.h"
#include "console/print_out.h"
#include "arm_vec.h"

extern void arm_vector_reset_entry(void);
extern void arm_vector_undefined_entry(void);
extern void arm_vector_prefetch_abort_entry(void);
extern void arm_vector_data_abort_entry(void);
extern void arm_vector_reserved_entry(void);
extern void arm_vector_irq_entry(void);
extern void arm_vector_fiq_entry(void);
extern void arm_vector_system_call_entry(void);

// C handlers called by assembly entries
static arm_vec_handler_t *vector_table[8];

#define DEFINE_HANDLER(name, vec) \
void name##_handler(void) { \
    register uint64_t context asm("sp"); \
    vector_table[vec](vec, context); \
}

DEFINE_HANDLER(arm_vector_reset, VECTOR_RESET)
DEFINE_HANDLER(arm_vector_undefined, VECTOR_UNDEFINED)
DEFINE_HANDLER(arm_vector_prefetch_abort, VECTOR_PRETECH_ABORT)
DEFINE_HANDLER(arm_vector_data_abort, VECTOR_DATA_ABORT)
DEFINE_HANDLER(arm_vector_reserved, VECTOR_RESERVED)
DEFINE_HANDLER(arm_vector_irq, VECTOR_IRQ)
DEFINE_HANDLER(arm_vector_fiq, VECTOR_FIQ)
DEFINE_HANDLER(arm_vector_system_call, VECTOR_SYSTEM_CALL)

void arm_vec_install(arm_vec_t vector, arm_vec_handler_t *handler)
{
    vector_table[vector] = handler;
    volatile uint64_t *vector_root = (uint64_t *)0x0; // VBAR_EL1

    switch(vector)
    {
        case VECTOR_RESET:         vector_root[vector] = (uint64_t)&arm_vector_reset_entry; break;
        case VECTOR_UNDEFINED:     vector_root[vector] = (uint64_t)&arm_vector_undefined_entry; break;
        case VECTOR_PRETECH_ABORT: vector_root[vector] = (uint64_t)&arm_vector_prefetch_abort_entry; break;
        case VECTOR_DATA_ABORT:    vector_root[vector] = (uint64_t)&arm_vector_data_abort_entry; break;
        case VECTOR_RESERVED:      vector_root[vector] = (uint64_t)&arm_vector_reserved_entry; break;
        case VECTOR_IRQ:           vector_root[vector] = (uint64_t)&arm_vector_irq_entry; break;
        case VECTOR_FIQ:           vector_root[vector] = (uint64_t)&arm_vector_fiq_entry; break;
        case VECTOR_SYSTEM_CALL:   vector_root[vector] = (uint64_t)&arm_vector_system_call_entry; break;
        default: return;
    }
}
