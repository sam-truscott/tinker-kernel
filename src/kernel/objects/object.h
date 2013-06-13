/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef OBJECT_H_
#define OBJECT_H_

#include "../kernel_types.h"

void __obj_initialise_object(__object_t * const o);

bool __obj_is_initialised(__object_t * const o);

void __obj_set_allocated(
		__object_t * const o,
		const bool a );

bool __obj_is_allocated(__object_t * const o);

uint32_t __obj_get_number(__object_t * const o);

void __obj_set_number(__object_t * const o, const uint32_t on);

__object_type_t __obj_get_type(__object_t * const o);

void __obj_set_type(
		__object_t * const o,
		const __object_type_t ot);

void __obj_lock(__object_t * const o);

void __obj_release(__object_t * const o);

void __obj_increase_ref_count(__object_t * const o);

void __obj_decrease_ref_count(__object_t * const o);

uint32_t __obj_get_ref_count(const __object_t * const o);

#endif /* OBJECT_H_ */
