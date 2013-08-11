/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
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
		const uint32_t messages) SOS_API_SUFFIX;

error_t sos_open_pipe(
		sos_pipe_t * pipe,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages) SOS_API_SUFFIX;

error_t sos_close_pipe(sos_pipe_t * pipe) SOS_API_SUFFIX;

error_t sos_delete_pipe(sos_pipe_t * pipe) SOS_API_SUFFIX;

error_t sos_send_message(
		sos_pipe_t * pipe,
		const sos_pipe_send_kind_t send_kend,
		void * const message,
		const uint32_t message_size,
		const bool_t block) SOS_API_SUFFIX;

error_t sos_receive_message(
		sos_pipe_t * pipe,
		const void ** message,
		const uint32_t * const message_size,
		const bool_t block) SOS_API_SUFFIX;

error_t sos_received_message(sos_pipe_t * pipe) SOS_API_SUFFIX;

#endif /* SOS_API_PIPE_H_ */
