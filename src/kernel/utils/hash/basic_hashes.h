/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef BASIC_HASHES_H_
#define BASIC_HASHES_H_

#include "hash.h"

int32_t __hash_basic_integer(const void * ptr, const uint32_t size);

int32_t __hash_basic_string(const void * ptr, const uint32_t size);

#endif /* BASIC_HASHES_H_ */
