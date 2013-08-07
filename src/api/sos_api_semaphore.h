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

typedef object_number_t sos_sem;

error_t sos_sem_create(sos_sem * semaphore, uint32_t initial_count, char * name) SOS_API_SUFFIX;

error_t sos_sem_get(sos_sem semaphore) SOS_API_SUFFIX;

error_t sos_sem_release(sos_sem semaphore) SOS_API_SUFFIX;


#endif /* SOS_API_SEMAPHORE_H_ */
