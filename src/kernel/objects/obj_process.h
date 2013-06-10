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

#ifndef OBJ_PROCESS_H_
#define OBJ_PROCESS_H_

#include "../kernel_types.h"

error_t __obj_create_process(
		__mem_pool_info_t * pool,
		__object_table_t * table,
		const uint32_t process_id,
		__object_t ** object);

#endif /* OBJ_PROCESS_H_ */
