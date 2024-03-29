/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJ_PIPE_H_
#define OBJ_PIPE_H_

#include "tinker_api_pipe.h"
#include "process/process.h"
#include "objects/object.h"
#include "objects/obj_thread.h"
#include "objects/object_table.h"
#include "objects/registry.h"

typedef struct object_pipe_t object_pipe_t;

object_pipe_t * obj_cast_pipe(void * const o);

object_number_t obj_pipe_get_oid
	(const object_pipe_t * const o);

return_t obj_create_pipe(
		registry_t * const reg,
		process_t * const process,
		object_number_t * objectno,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages);

return_t obj_open_pipe(
		registry_t * const reg,
		process_t * const process,
		object_thread_t * const thread,
		object_number_t * objectno,
		const char * const name,
		const tinker_pipe_direction_t direction,
		const uint32_t message_size,
		const uint32_t messages);

return_t obj_delete_pipe(object_pipe_t * const pipe);

return_t obj_pipe_send_message(
		object_pipe_t * const pipe,
		object_thread_t * const thread,
		const tinker_pipe_send_kind_t send_kind,
		void * const message,
		const uint32_t message_size,
		const bool_t block);

return_t obj_pipe_receive_message(
		object_pipe_t * const pipe,
		object_thread_t * const thread,
		void * const message,
		uint32_t * const message_size,
		uint32_t max_size,
		const bool_t block);

const char * obj_pipe_get_name(const object_pipe_t * const pipe);

tinker_pipe_direction_t obj_pipe_get_direction(const object_pipe_t * const pipe);

uint32_t obj_pipe_get_total_messages(const object_pipe_t * const pipe);

uint32_t obj_pipe_get_free_messages(const object_pipe_t * const pipe);

uint32_t obj_pipe_get_msg_size(const object_pipe_t * const pipe);

uint32_t obj_pipe_get_read_msg_pos(const object_pipe_t * const pipe);

uint32_t obj_pipe_get_write_msg_pos(const object_pipe_t * const pipe);

#endif /* OBJ_PIPE_H_ */
