/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "tinker_api_process.h"
#include "tinker_api.h"
#include "tinker_api_kernel_interface.h"

/**
 * Create a new Process
 * @param image_name The name of the process
 * @param initial_thread_name The name of the initial thread/task
 * @param entry The entry point of the initial thread/task
 * @param priority The priority of the initial thread/task
 * @param heap The amount of heap for the process. This will be taken from the
 * parent process. If the parent process does not have enough spare heap for
 * this new process to be created then it will fail.
 * @param stack The amount of stack for th einitial threa/task.
 * @param process A pointer to the created Process.
 * @param thread A pointer to the created Thread.
 * @return
 */
return_t tinker_create_process(
		const char * image_name,
		thread_entry_point * entry,
		uint8_t priority,
		const tinker_meminfo_t * const meminfo,
		uint32_t flags,
		tinker_process_t * process)
{
	return TINKER_API_CALL_6(
			SYSCALL_CREATE_PROCESS,
			(uint32_t)image_name,
			(uint32_t)entry,
			(uint32_t)priority,
			(uint32_t)meminfo,
			flags,
			(uint32_t)process);
}

return_t tinker_create_thread(
		const char * thread_name,
		thread_entry_point * entry,
		uint8_t priority,
		uint32_t stack,
		uint32_t flags,
		tinker_thread_t * thread)
{
	return TINKER_API_CALL_6(
			SYSCALL_CREATE_THREAD,
			(uint32_t)thread_name,
			(uint32_t)entry,
			(uint32_t)priority,
			stack,
			flags,
			(uint32_t)thread);
}

return_t tinker_get_thread_object(tinker_thread_t * thread)
{
	return TINKER_API_CALL_1(
			SYSCALL_THREAD_OBJECT,
			(uint32_t)thread);
}

return_t tinker_get_thread_priority(
		tinker_thread_t thread,
		uint8_t * priority)
{
	return TINKER_API_CALL_2(
			SYSCALL_THREAD_PRIORITY,
			(uint32_t)thread,
			(uint32_t)priority);
}

void tinker_thread_wait(void)
{
	TINKER_API_CALL_0(SYSCALL_WAIT_THREAD);
}

return_t tinker_exit_thread(void)
{
	return TINKER_API_CALL_0(SYSCALL_EXIT_THREAD);
}

void tinker_debug(const char * const str, int len)
{
	TINKER_API_CALL_2(SYSCALL_DEBUG, (uint32_t)str, len);
}

void tinker_wait_for_interrupt(void)
{
	TINKER_API_CALL_0(SYSCALL_WFI);
}

return_t tinker_map_mempry(
		void * real,
		uint32_t size,
		tinker_memory_t type,
		tinker_privilege_t access,
		void ** virtual)
{
	return TINKER_API_CALL_5(SYSCALL_MMAP, (uint32_t)real, size, type, access, (uint32_t)virtual);
}

void * tinker_sbrk(void * memory, uint32_t size)
{
	void ** memory_addr = &memory;
	TINKER_API_CALL_2(SYSCALL_SBRK, (uint32_t)memory_addr, size);
	return *memory_addr;
}

return_t tinker_load_elf(
		void * elf_data,
		char * const image,
		uint8_t priority,
		uint32_t flags)
{
	return TINKER_API_CALL_4(SYSCALL_LOAD_ELF, (uint32_t)elf_data, (uint32_t)image, priority, flags);
}

int32_t tinker_get_pid(void)
{
	int32_t pid = -1;
	TINKER_API_CALL_1(SYSCALL_GET_PID, (uint32_t)&pid);
	return pid;
}
