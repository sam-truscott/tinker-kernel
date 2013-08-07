/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef REGISTRY_H_
#define REGISTRY_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"
#include "kernel/process/process.h"

void __registry_initialise(__mem_pool_info_t * const pool);

error_t __regsitery_add(
		const char * const name,
		const __process_t * const process,
		const object_number_t number);

error_t __registry_get(
		const char * const name,
		__process_t ** process,
		object_number_t * objno);

error_t __registry_remove(const char * const name);

#endif /* REGISTRY_H_ */
