/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "sos_api_pipe.h"

#include "sos_api_kernel_interface.h"

error_t sos_create_pipe(
		sos_pipe_t * pipe,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	return TINKER_API_CALL_5(
			SYSCALL_CREATE_PIPE,
			(uint32_t)pipe,
			(uint32_t)name,
			direction,
			message_size,
			messages);
}

error_t sos_open_pipe(
		sos_pipe_t * pipe,
		const char * const name,
		const sos_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages)
{
	error_t result = BLOCKED_RETRY;
	while(result == BLOCKED_RETRY)
	{
		result = TINKER_API_CALL_5(
			SYSCALL_OPEN_PIPE,
			(uint32_t)pipe,
			(uint32_t)name,
			direction,
			message_size,
			messages);
	}
	return result;
}

error_t sos_close_pipe(sos_pipe_t pipe)
{
	return TINKER_API_CALL_1(
			SYSCALL_CLOSE_PIPE,
			(uint32_t)pipe);
}

error_t sos_delete_pipe(sos_pipe_t pipe)
{
	return TINKER_API_CALL_1(
			SYSCALL_DELETE_PIPE,
			(uint32_t)pipe);
}

error_t sos_send_message(
		sos_pipe_t pipe,
		const sos_pipe_send_kind_t send_kend,
		void * const message,
		const uint32_t message_size,
		const bool_t block)
{
	error_t result = BLOCKED_RETRY;
	while(result == BLOCKED_RETRY)
	{
		result = TINKER_API_CALL_5(
				SYSCALL_SEND_MESSAGE,
				(uint32_t)pipe,
				send_kend,
				(uint32_t)message,
				(uint32_t)message_size,
				block);
	}
	return result;
}

error_t sos_receive_message(
		sos_pipe_t pipe,
		const void ** message,
		const uint32_t * const message_size,
		const bool_t block)
{
	return TINKER_API_CALL_4(
			SYSCALL_RECEIVE_MESSAGE,
			(uint32_t)pipe,
			(uint32_t)message,
			(uint32_t)message_size,
			block);
}

error_t sos_received_message(sos_pipe_t pipe)
{
	return TINKER_API_CALL_1(
			SYSCALL_RECEIVED_MESSAGE,
			(uint32_t)pipe);
}

