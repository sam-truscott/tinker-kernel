/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_SHM_H_
#define TINKER_API_SHM_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"

typedef object_number_t sos_shm_t;

error_t sos_shm_create(sos_shm_t * shm, char * name, uint32_t size, void** addr) TINKER_API_SUFFIX;

error_t sos_shm_open(sos_shm_t * shm, char * name, uint32_t size, void** addr) TINKER_API_SUFFIX;

error_t sos_shm_destroy(sos_shm_t shm) TINKER_API_SUFFIX;


#endif /* TINKER_API_SHM_H_ */
