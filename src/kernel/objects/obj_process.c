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

#include "obj_process.h"
#include "object.h"
#include "object_table.h"

error_t __obj_create_process(
		__mem_pool_info_t * pool,
		__object_table_t * table,
		const uint32_t process_id,
		__object_t ** object)
{
	__object_t * no = NULL;
	error_t result = NO_ERROR;

	if ( object )
	{
		if ( table)
		{
			if ( (result = __obj_allocate_next_free_object(pool, table, &no)) == NO_ERROR )
			{
				__obj_set_type(no, PROCESS_OBJ);
				__obj_lock(no);
				no->specifics.process.pid = process_id;
				__obj_release(no);
				*object = no;
			}
		}
		else
		{
			result = PARAMETERS_OUT_OF_RANGE;
		}
	}
	else
	{
		result = PARAMETERS_NULL;
	}

	return result;
}
