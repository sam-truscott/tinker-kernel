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

#include "arch/tgt_types.h"

int32_t __hash_basic_integer(const void * ptr, const uint32_t size);

int32_t __hash_basic_string(const void * ptr, const uint32_t size);

bool_t __hash_equal_integer(const uint32_t l, const uint32_t r);

bool_t __hash_equal_string(const char * const l, const char * const r);

#endif /* BASIC_HASHES_H_ */
