/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef REGISTRY_H_
#define REGISTRY_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"
#include "config.h"
#include "process/process.h"
#include "objects/obj_thread.h"

typedef struct registry_t registry_t;

registry_t * registry_create(mem_pool_info_t * const pool);

return_t regsitery_add(
		registry_t * const reg,
		const char * const name,
		const process_t * const process,
		const object_number_t number);

return_t registry_get(
		registry_t * const reg,
		const char * const name,
		process_t ** process,
		object_number_t * objno);

void registry_wait_for(
		registry_t * const reg,
		object_thread_t * const thread,
		const char * const name);

return_t registry_remove(
		registry_t * const reg,
		const char * const name);

#endif /* REGISTRY_H_ */
