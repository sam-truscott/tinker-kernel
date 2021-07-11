/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_LIST_ITERATOR_H_
#define UNBOUNDED_LIST_ITERATOR_H_

#include "tinker_api_types.h"
#include "utils/collections/unbounded_list.h"

typedef struct list_it_t list_it_t;

list_it_t * list_it_create(const list_t * const list);
void list_it_initialise(list_it_t * const it, const list_t * const list);
void list_it_delete(list_it_t * const it);
bool_t list_it_get(list_it_t * const it, void * item);
bool_t list_it_fwd(list_it_t * const it, uint32_t count);
bool_t list_it_back(list_it_t * const it, uint32_t count);
bool_t list_it_next(list_it_t * const it, void * item);
bool_t list_it_prev(list_it_t * const it, void * item);
void list_it_reset(list_it_t * const it);
uint32_t list_it_where(list_it_t const * it);

#endif /* UNBOUNDED_LIST_ITERATOR_H_ */
