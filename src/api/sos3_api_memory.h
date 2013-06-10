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

#ifndef SOS2_API_MEMORY_H_
#define SOS2_API_MEMORY_H_

#include "sos3_api_types.h"

void *	sos2_memory_malloc(uint32_t size) SOS3_API_SUFFIX;

void	sos2_memory_free(void * ptr) SOS3_API_SUFFIX;

#endif /* SOS2_API_MEMORY_H_ */
