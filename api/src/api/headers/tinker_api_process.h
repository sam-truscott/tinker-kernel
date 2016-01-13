/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TINKER_API_PROCESS_H_
#define TINKER_API_PROCESS_H_

#include "tinker_api_types.h"
#include "tinker_api_errors.h"
#include "tinker_api_kernel_interface.h"

/* needs to match mmu_memory_t */
typedef enum
{
	MEM_DEVICE_MEMORY		 	= 0,
	MEM_RANDOM_ACCESS_MEMORY	= 1
} tinker_memory_t;

/* needs to match mmu_privilege_t */
typedef enum
{
	MEM_NO_PRIVILEGE 	= 0,
	MEM_USER_ACCESS		= 1,
	MEM_KERNEL_ACCESS	= 2,
	MEM_ALL_ACCESS		= 3
} tinker_privilege_t;

/* needs to match mmu_access_t */
typedef enum
{
	MEM_NO_ACCESS	= 0,
	MEM_READ_ONLY	= 1,
	MEM_READ_WRITE	= 2
} tinker_access_t;

/**
 * The entry point for a new thread
 */
typedef void (thread_entry_point)(void);

/**
 * The type in user space for a process
 */
typedef object_number_t tinker_process_t;

/**
 * The type in user space for a thread
 */
typedef object_number_t tinker_thread_t;

typedef struct tinker_mempart
{
	uint32_t real;
	uint32_t virt;
	uint32_t size;
	tinker_memory_t mem_type;
	tinker_privilege_t priv;
	tinker_access_t access;
	struct tinker_mempart * next;
} tinker_mempart_t;

typedef struct tinker_meminfo {
	uint32_t heap_size;
	uint32_t stack_size;
	tinker_mempart_t * first_part;
} tinker_meminfo_t;

error_t tinker_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		const tinker_meminfo_t * const meminfo,
		uint32_t flags,
		tinker_process_t * process) TINKER_API_SUFFIX;

error_t tinker_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		tinker_thread_t * thread) TINKER_API_SUFFIX;

error_t tinker_get_thread_object(tinker_thread_t * thread) TINKER_API_SUFFIX ;

error_t	tinker_get_thread_priority(
		tinker_thread_t thread,
		uint8_t * priority) TINKER_API_SUFFIX;

void tinker_thread_wait(void);

error_t tinker_exit_thread(void) TINKER_API_SUFFIX;

void tinker_debug(const char * const str) TINKER_API_SUFFIX;

void tinker_wait_for_interrupt(void) TINKER_API_SUFFIX;

error_t tinker_map_mempry(
		void * real,
		uint32_t size,
		tinker_memory_t type,
		tinker_privilege_t access,
		void ** virtual);

void * tinker_sbrk(void * memory, uint32_t size);

#endif /* TINKER_API_PROCESS_H_ */
