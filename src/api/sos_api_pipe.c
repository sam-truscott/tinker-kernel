/*
 *
 * SOS Source Code
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
	return SOS_API_CALL_5(
			syscall_create_pipe,
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
		result = SOS_API_CALL_5(
			syscall_open_pipe,
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
	return SOS_API_CALL_1(
			syscall_close_pipe,
			(uint32_t)pipe);
}

error_t sos_delete_pipe(sos_pipe_t pipe)
{
	return SOS_API_CALL_1(
			syscall_delete_pipe,
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
		result = SOS_API_CALL_5(
				syscall_send_message,
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
	return SOS_API_CALL_4(
			syscall_receive_message,
			(uint32_t)pipe,
			(uint32_t)message,
			(uint32_t)message_size,
			block);
}

error_t sos_received_message(sos_pipe_t pipe)
{
	return SOS_API_CALL_1(
			syscall_received_message,
			(uint32_t)pipe);
}

