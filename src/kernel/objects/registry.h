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
#include "kernel/process/process.h"
#include "kernel/objects/obj_thread.h"

typedef char registry_key_t[MAX_SHARED_OBJECT_NAME_LENGTH];

void registry_initialise(mem_pool_info_t * const pool);

error_t regsitery_add(
		const char * const name,
		const process_t * const process,
		const object_number_t number);

error_t registry_get(
		const char * const name,
		process_t ** process,
		object_number_t * objno);

void registry_wait_for(
		object_thread_t * const thread,
		const char * const name);

error_t registry_remove(const char * const name);

#endif /* REGISTRY_H_ */
