/*
 * sos_api_pipe.h
 *
 *  Created on: 9 Aug 2013
 *      Author: sam
 */

#ifndef SOS_API_PIPE_H_
#define SOS_API_PIPE_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"

typedef object_number_t sos_pipe_t;

typedef enum sos_pipe_direction
{
	PIPE_DIRECTION_UNKNOWN = 0,
	PIPE_SEND_RECEIVE,
	PIPE_SEND,
	PIPE_RECEIVE
} sos_pipe_direction_t;

typedef enum sos_pipe_send_kind
{
	PIPE_TX_SEND_ALL,
	PIPE_TX_SEND_AVAILABLE
} sos_pipe_send_kind_t;

// TODO: Add timeouts to the open, send and receive calls

error_t sos_create_pipe(
		sos_pipe_t * pipe,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages);

error_t sos_open_pipe(
		sos_pipe_t * pipe,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages);

error_t sos_close_pipe(sos_pipe_t * pipe);

error_t sos_delete_pipe(sos_pipe_t * pipe);

error_t sos_send_message(
		sos_pipe_t * pipe,
		const sos_pipe_send_kind_t send_kend,
		void * const message,
		const uint32_t message_size,
		const bool_t block);

error_t sos_receive_message(
		sos_pipe_t * pipe,
		const void ** message,
		const uint32_t * const message_size,
		const bool_t block);

error_t sos_received_message(sos_pipe_t * pipe);

#endif /* SOS_API_PIPE_H_ */
