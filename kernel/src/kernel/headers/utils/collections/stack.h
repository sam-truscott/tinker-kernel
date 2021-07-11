/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef STACK_H_
#define STACK_H_

#include "tinker_api_types.h"

typedef struct stack_t stack_t;

list_t * stack_list(stack_t * const stack);
void stack_initialise(stack_t * const stack, mem_pool_info_t * const pool);
stack_t * stack_create(mem_pool_info_t * const pool);
void stack_delete(stack_t * const stack);
bool stack_push(stack_t * const stack, void * const item);
bool_t stack_insert(stack_t * const stack, const uint32_t index, void * const item);
bool_t stack_pop(stack_t * const stack, void * const item);
bool_t stack_front(const stack_t * const stack, void * const item);
bool_t stack_get(const stack_t * const stack, const uint32_t index, void * const item);
stack_size(const stack_t * const stack);

#endif /* STACK_H_ */
