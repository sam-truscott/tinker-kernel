/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_SEMAPHORE_H_
#define SOS_API_SEMAPHORE_H_

#include "sos_api_types.h"
#include "sos_api_errors.h"

typedef object_number_t sos_sem_t;

error_t sos_sem_create(sos_sem_t * semaphore, uint32_t initial_count, char * name) SOS_API_SUFFIX;

error_t sos_sem_open(sos_sem_t * semaphore, char * name) SOS_API_SUFFIX;

error_t sos_sem_get(sos_sem_t semaphore) SOS_API_SUFFIX;

error_t sos_sem_release(sos_sem_t semaphore) SOS_API_SUFFIX;

error_t sos_sem_close(sos_sem_t semaphore) SOS_API_SUFFIX;

#endif /* SOS_API_SEMAPHORE_H_ */
