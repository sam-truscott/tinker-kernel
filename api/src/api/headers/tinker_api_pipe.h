/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_PIPE_H_
#define TINKER_API_PIPE_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"

typedef object_number_t tinker_pipe_t;

typedef enum tinker_pipe_direction
{
	PIPE_DIRECTION_UNKNOWN = 0,
	PIPE_SEND_RECEIVE,
	PIPE_SEND,
	PIPE_RECEIVE
} tinker_pipe_direction_t;

typedef enum tinker_pipe_send_kind
{
	PIPE_TX_SEND_ALL,
	PIPE_TX_SEND_AVAILABLE
} tinker_pipe_send_kind_t;

// TODO: Add timeouts to the open, send and receive calls

error_t tinker_create_pipe(
        tinker_pipe_t * pipe,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages) TINKER_API_SUFFIX;

error_t tinker_open_pipe(
        tinker_pipe_t * pipe,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages) TINKER_API_SUFFIX;

error_t tinker_close_pipe(tinker_pipe_t pipe) TINKER_API_SUFFIX;

error_t tinker_delete_pipe(tinker_pipe_t pipe) TINKER_API_SUFFIX;

error_t tinker_send_message(
        tinker_pipe_t pipe,
		const tinker_pipe_send_kind_t send_kend,
		void * const message,
		const uint32_t message_size,
		const bool_t block) TINKER_API_SUFFIX;

error_t tinker_receive_message(
        tinker_pipe_t pipe,
		void ** message,
		uint32_t ** const message_size,
		const bool_t block) TINKER_API_SUFFIX;

error_t tinker_received_message(tinker_pipe_t pipe) TINKER_API_SUFFIX;

#endif /* TINKER_API_PIPE_H_ */
