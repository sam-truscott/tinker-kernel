/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef UNBOUNDED_QUEUE_H_
#define UNBOUNDED_QUEUE_H_

#include "tinker_api_types.h"

#include "memory/memory_manager.h"

typedef struct queue_t queue_t;

void queue_initialise(queue_t * const queue, mem_pool_info_t * const pool);
queue_t * queue_create(mem_pool_info_t * const pool);
void queue_delete(queue_t * const queue);
bool_t queue_push(queue_t * const queue, void * const item);
bool_t queue_pop(queue_t * const queue);
bool_t queue_front(const queue_t * const queue, void * const item_ptr);
bool_t queue_reorder_first(queue_t * const queue);
uint32_t queue_size(const queue_t * const queue);
bool_t queue_remove(queue_t * const queue, void * const item);
bool_t queue_contains(queue_t * const queue, void * item);

#endif /* UNBOUNDED_QUEUE_H_ */
