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

#ifndef OBJECT_TABLE_H_
#define OBJECT_TABLE_H_

#include "../kernel_types.h"
#include "../utils/collections/hashed_map.h"

error_t __obj_initialse_table(__process_t * proc, __object_table_t * t);

error_t __obj_allocate_next_free_object(
		__mem_pool_info_t * pool,
		__object_table_t * t,
		__object_t ** o);

__object_t * __obj_get_thread_object(__object_table_t * t, uint32_t oid);

#endif /* OBJECT_TABLE_H_ */
