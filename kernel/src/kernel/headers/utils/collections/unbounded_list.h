/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_LIST_H_
#define UNBOUNDED_LIST_H_

#include "tinker_api_types.h"
#include "memory/memory_manager.h"

typedef struct list_t list_t;

typedef struct list_it_t list_it_t;

/**
 * This abstract data type is used to model a generic
 * single (forward) linked list.
 *
 * The list must first be created or, if static, initialised using
 * _create or _initialise.
 *
 * The user can then use _add, _get and _remove to modify the
 * values in the list.
 *
 * The size of the list can be queried with _size.
 *
 * Once the user has finished, and if the list was created dynamically
 * with the _create function, the _delete procedure should be used
 * to free the memory on the heap.
 */

list_t * list_create(mem_pool_info_t * const pool);
void list_initialise(list_t * const list, mem_pool_info_t * const pool);
void list_delete(list_t * const list);
bool_t list_insert(list_t * const list, const uint32_t index, void * const item);
uint32_t list_size(const list_t * const list);
bool_t list_get(const list_t * const list, const uint32_t index, void * const item_ptr);
bool_t list_contains(list_t * const list, void * const item);

bool_t list_get_tail(const list_t * const list, void * const item_ptr);
bool_t list_next(const list_t * const list, const void * current, void ** const next_ptr);
bool_t list_remove_tail(list_t * const list);
bool_t list_head_to_tail(list_t * const list);

/**
 * Add an entry to the list
 */
bool_t list_add(list_t * const list, void * const item);


/**
 * Delete an entry from the list
 */
bool_t list_remove(list_t * const list, const uint32_t index);
bool_t list_remove_item(list_t * const list, void * const item);

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

#endif /* UNBOUNDED_LIST_H_ */
