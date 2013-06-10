/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef BASIC_HASHES_H_
#define BASIC_HASHES_H_

#include "hash.h"

int32_t __hash_basic_integer(const void * ptr, const uint32_t size);

int32_t __hash_basic_string(const void * ptr, const uint32_t size);

#endif /* BASIC_HASHES_H_ */
