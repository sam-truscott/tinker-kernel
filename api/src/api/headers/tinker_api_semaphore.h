/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_SEMAPHORE_H_
#define TINKER_API_SEMAPHORE_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"

typedef object_number_t tinker_sem_t;

return_t tinker_sem_create(tinker_sem_t * semaphore, uint32_t initial_count, char * name) TINKER_API_SUFFIX;

return_t tinker_sem_open(tinker_sem_t * semaphore, char * name) TINKER_API_SUFFIX;

return_t tinker_sem_get(tinker_sem_t semaphore) TINKER_API_SUFFIX;

return_t tinker_sem_release(tinker_sem_t semaphore) TINKER_API_SUFFIX;

return_t tinker_sem_close(tinker_sem_t semaphore) TINKER_API_SUFFIX;

#endif /* TINKER_API_SEMAPHORE_H_ */
