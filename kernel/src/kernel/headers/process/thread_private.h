/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_PROCESS_THREAD_PRIVATE_H_
#define KERNEL_PROCESS_THREAD_PRIVATE_H_

#include "config.h"
#include "tgt_types.h"
#include "objects/object.h"
#include "process/process.h"

typedef struct thread_t
{
	uint32_t			thread_id;
	void 				* stack;
	uint32_t			r_stack_base;
	uint32_t			v_stack_base;
	uint32_t			stack_size;
	priority_t			priority;
	uint32_t			flags;
	process_t 		*	parent;
	thread_state_t	state;
	const object_t *	waiting_on;
	thread_entry_point * 	entry_point;
	object_number_t		object_number;
	tgt_context_t		* context;
	char 				name[MAX_THREAD_NAME_LEN + 1];
} thread_internal_t;

#endif /* KERNEL_PROCESS_THREAD_PRIVATE_H_ */
