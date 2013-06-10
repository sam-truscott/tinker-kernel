/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef SOS_API_MEMORY_H_
#define SOS_API_MEMORY_H_

#include "sos3_api_types.h"

void *	sos_memory_malloc(uint32_t size) SOS_API_SUFFIX;

void	sos_memory_free(void * ptr) SOS_API_SUFFIX;

#endif /* SOS_API_MEMORY_H_ */
