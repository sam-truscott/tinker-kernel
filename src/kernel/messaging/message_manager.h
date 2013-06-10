/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef MESSAGE_MANAGER_H_
#define MESSAGE_MANAGER_H_

#include "kernel/kernel_types.h"

void __msg_initialise(void);

void __msg_create_pipe(
		__object_t * pipe,
		/* TODO urgh can't figure this out yet */);

void __msg_send_message(
		__object_t * pipe,
		void * message,
		uint32_t message_size);

void __msg_receive_message(
		__object_t * pipe,
		void * message_buffer,
		uint32_t message_buffer_size,
		uint32_t* message_buffer_received);

void __msg_delete_pipe(
		__object_t * pipe);

#endif /* MESSAGE_MANAGER_H_ */
