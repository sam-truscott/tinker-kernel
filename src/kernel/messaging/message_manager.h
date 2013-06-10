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
		UINT32 message_size);

void __msg_receive_message(
		__object_t * pipe,
		void * message_buffer,
		UINT32 message_buffer_size,
		UINT32* message_buffer_received);

void __msg_delete_pipe(
		__object_t * pipe);

#endif /* MESSAGE_MANAGER_H_ */
