/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
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

#ifndef DEBUG_STACK_TRACE_H_
#define DEBUG_STACK_TRACE_H_

#include "kernel/kernel_types.h"

void __debug_print_current_stack_trace(void);

void __debug_print_stack_trace(uint32_t frame_pointer);

#endif /* DEBUG_STACK_TRACE_H_ */
