/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef DEBUG_STACK_TRACE_H_
#define DEBUG_STACK_TRACE_H_

#include "arch/tgt_types.h"
#include "kernel/process/process.h"

void print_stack_trace(process_t * const proc, const uint32_t frame_pointer);

#endif /* DEBUG_STACK_TRACE_H_ */
