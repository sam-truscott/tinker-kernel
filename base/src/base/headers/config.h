/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * Linker section to use for boot code.
 *
 * Placed prior to standard code at the start of the binary file
 */
#define BOOT_CODE __attribute__((section(".boot")))

/**
 * Deprecated - can probably be removed.
 *
 * Defines the maximum number of system call arguments supported
 */
#define MAX_SYSCALL_ARGS 7

/**
 * Base memory address of a virtual process
 */
#define USER_ADDRESS_SPACE 0xC0000000u
#define KERNEL_ADDRESS_SPACE 0x80000000u
#define VIRTUAL_ADDRESS_SPACE(K) \
	(K ? KERNEL_ADDRESS_SPACE : USER_ADDRESS_SPACE)

/**
 * Maximum number of stack frames to print on a fatal thread exception
 */
#define ISR_PRINT_STACKTRACE_LIMIT 6

/**
 * Hertz of the scheduler
 */
#define SCHEDULER_HERTZ 10

/**
 * The size of the heap for the kernel
 */
#define KERNEL_HEAP 0x100000

/**
 * The size of the stack for the Kernels idle thread
 */
#define KERNEL_IDLE_STACK 0x400

/**
 * The priority of the idle process - should remain as zero
 */
#define KERNEL_IDLE_PRIORITY 0

/**
 * The size of the private (internal) memory pool.
 *
 * This is used to store things such as data structures and the mmu page table
 * which shouldn't be mapped/visible to the user process
 */
#define PRIVATE_POOL_SIZE (256 * 1024)

/**
 * The maximum number of processes allowed in the system
 */
#define MAX_PROCESSES 1024
/**
 * The maximum number of threads allowed per process
 */
#define MAX_THREADS 1024
/**
 * The maximum thread priority of the scheduler
 */
#define MAX_PRIORITY 255

/**
 * The name of the thread that's the entry point of the process
 */
#define MAIN_THREAD_NAME "main"

/**
 * The maximum length of the name of a thread
 */
#define MAX_THREAD_NAME_LEN 255
/**
 * The maximum length of the name of a process
 */
#define MAX_PROCESS_IMAGE_LEN 255

/**
 * The maximum number of objects allowed in an object table
 */
#define MAX_OBJECT_TABLE_SIZE 1024

/**
 * The maximum number of shared (named) objects in the registry
 */
#define MAX_SHARED_OBJECTS (MAX_OBJECT_TABLE_SIZE * MAX_PROCESSES)

/**
 * The maximum length of the name of a shared object
 */
#define MAX_SHARED_OBJECT_NAME_LENGTH 64

/**
 * The stack size for timers TODO replace with argument?
 */
#define TIMER_STACK_SIZE 1024

/**
 * The maximum number of external interrupt vectors handled
 * by the master/slave interrupt controllers
 */
#define MAX_ISRS 256

/**
 * The maximum number of alarms the system can support
 */
#define MAX_ALARMS 256

/**
 * The maximum amount of padding to allow in printf-based (printp_out, debug_print)
 */
#define MAX_PADDING 256

#endif /* CONFIG_H_ */
