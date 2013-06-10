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

#ifndef SOS2_API_SEMAPHORE_H_
#define SOS2_API_SEMAPHORE_H_

#include "sos3_api_types.h"
#include "sos3_api_errors.h"

typedef void* sos2_sem;

error_t sos2_sem_create(sos2_sem * semaphore, uint32_t initial_count) SOS3_API_SUFFIX;

error_t sos2_sem_get(sos2_sem semaphore) SOS3_API_SUFFIX;

error_t sos2_sem_release(sos2_sem semaphore) SOS3_API_SUFFIX;


#endif /* SOS2_API_SEMAPHORE_H_ */
