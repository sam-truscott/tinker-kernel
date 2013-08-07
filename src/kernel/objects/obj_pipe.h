/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_PIPE_H_
#define OBJ_PIPE_H_

#include "kernel/process/process.h"
#include "kernel/objects/object.h"
#include "kernel/objects/object_table.h"
#include "kernel/objects/registry.h"

typedef enum __object_pipe_direction
{
	PIPE_DIRECTION_UNKNOWN = 0,
	PIPE_SEND_RECEIVE,
	PIPE_SEND,
	PIPE_RECEIVE
} __object_pipe_direction_t;

typedef struct __object_pipe_t __object_pipe_t;

__object_pipe_t * __obj_cast_semaphore(__object_t * o);

object_number_t __obj_pipe_get_oid
	(const __object_pipe_t * const o);

error_t __obj_create_pipe(
		__process_t * const process,
		__object_table_t * const table,
		object_number_t * objectno,
		const char * const name,
		const __object_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages);

error_t __obj_delete_pipe(__object_pipe_t * const pipe);

error_t __obj_pipe_send_message(
		__object_pipe_t * const pipe,
		void * const message,
		const uint32_t message_size,
		const bool_t block);

error_t __obj_pipe_receive_message(
		__object_pipe_t * const pipe,
		void * const message,
		uint32_t * const message_size,
		const uint32_t max_message_size,
		const bool_t block);

#endif /* OBJ_PIPE_H_ */
