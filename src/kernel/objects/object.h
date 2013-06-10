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

#ifndef OBJECT_H_
#define OBJECT_H_

#include "../kernel_types.h"

void __obj_initialise_object(__object_t * o);

bool __obj_is_initialised(__object_t * o);

void __obj_set_allocated(__object_t * o, const bool a );

bool __obj_is_allocated(__object_t * o);

uint32_t __obj_get_number(__object_t * o);

void __obj_set_number(__object_t * o, const uint32_t on);

__object_type_t __obj_get_type(__object_t * o);

void __obj_set_type(__object_t * o, const __object_type_t ot);

void __obj_lock(__object_t * o);

void __obj_release(__object_t * o);

void __obj_increase_ref_count(__object_t * o);

void __obj_decrease_ref_count(__object_t * o);

uint32_t __obj_get_ref_count(__object_t * o);

#endif /* OBJECT_H_ */
