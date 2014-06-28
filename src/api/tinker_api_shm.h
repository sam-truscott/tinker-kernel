/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_SHM_H_
#define TINKER_API_SHM_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"

typedef object_number_t tinker_shm_t;

error_t tinker_shm_create(tinker_shm_t * shm, char * name, uint32_t size, void** addr) TINKER_API_SUFFIX;

error_t tinker_shm_open(tinker_shm_t * shm, char * name, uint32_t size, void** addr) TINKER_API_SUFFIX;

error_t tinker_shm_destroy(tinker_shm_t shm) TINKER_API_SUFFIX;


#endif /* TINKER_API_SHM_H_ */
