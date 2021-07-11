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

/**
 * Add an entry to the list
 */
bool_t list_add(list_t * const list, void * const item);


/**
 * Delete an entry from the list
 */
bool_t list_remove(list_t * const list, const uint32_t index);
#endif /* UNBOUNDED_LIST_H_ */
