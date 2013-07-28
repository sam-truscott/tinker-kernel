/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "obj_pipe.h"

#include "arch/tgt_types.h"

typedef struct __object_pipe_t
{
	uint32_t pid_from;
	uint32_t pid_to;
	__object_pipe_direction_t direction;
} __object_pipe_internal_t;
