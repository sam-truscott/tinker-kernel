/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SYSCALL_HANDLER_H_
#define SYSCALL_HANDLER_H_

#include "arch/tgt_types.h"

void __syscall_handle_system_call(void * context, uint32_t context_size);

#endif /* SYSCALL_HANDLER_H_ */
