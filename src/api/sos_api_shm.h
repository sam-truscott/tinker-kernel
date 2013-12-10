/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_SHM_H_
#define SOS_API_SHM_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"

typedef object_number_t sos_shm_t;

error_t sos_shm_create(sos_shm_t * shm, char * name, uint32_t size, void** addr) SOS_API_SUFFIX;

error_t sos_shm_open(sos_shm_t * shm, char * name, uint32_t size, void** addr) SOS_API_SUFFIX;

error_t sos_shm_destroy(sos_shm_t shm) SOS_API_SUFFIX;


#endif /* SOS_API_SHM_H_ */
