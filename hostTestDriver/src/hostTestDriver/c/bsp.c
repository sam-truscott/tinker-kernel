/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2015] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "tinker_api_errors.h"
#include "memory/mem_pool.h"

#define HOST_MEMORY_SIZE 4 * 1024 * 1024

static mem_t memory = 0;

unsigned int __text = 0;
unsigned int __text_end = 0;
unsigned int __data_end = 0;
unsigned int __api = 0;
unsigned int __api_end = 0;

void TINKER_API_CALL_7()
{

}

void bsp_initialise()
{
	memory = (mem_t)malloc(HOST_MEMORY_SIZE);
}

void bsp_setup() {}

mem_t bsp_get_usable_memory_start()
{
	return (mem_t)memory;
}

mem_t bsp_get_usable_memory_end()
{
	return ((mem_t)memory) + HOST_MEMORY_SIZE;
}

void bsp_enable_schedule_timer() {}

unsigned int tgt_get_syscall_param()
{
	return 0;
}
void tgt_set_syscall_return() {}
tgt_pg_tbl_t* tgt_initialise_page_table(mem_pool_info_t * const pool) { return mem_alloc(pool, 4); }
void tgt_map_memory() {}
void tgt_unmap_memory() {}
return_t tgt_initialise_process() { return NO_ERROR; }
void tgt_load_context() {}
void tgt_save_context() {}
void tgt_initialise_context() {}
void tgt_set_context_param() {}
void tgt_disable_external_interrupts() {}
void tgt_enter_usermode() {}
void tgt_wait_for_interrupt() {raise(SIGABRT);}
void tgt_prepare_context() {}
void tgt_destroy_context() {}
void tgt_get_context_stack_pointer() {}
void bsp_write_debug_char(char c)
{
	printf("%c", c);
}

void lock_init() {}
void lock() {}
void unlock() {}
