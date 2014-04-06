/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#define MAX_SYSCALL_ARGS 7

#define VIRTUAL_ADDRESS_SPACE 0xC0000000u

#define ISR_PRINT_STACKTRACE_LIMIT 20

#define SCHEDULER_HERTZ 100

#define PAGE_TABLE_SIZE (1 * (1024 * 1024))

/**
 * The size of the heap for the kernel
 */
#define __KERNEL_HEAP 0x100000

/**
 * The size of the stack for the Kernels
 * idle thread
 */
#define __KERNEL_IDLE_STACK 0x400

#define __KERNEL_IDLE_PRIORITY 0

/**
 * Because they're defined on the heap we can allow as many as possible
 * but at some point memory might run out but we'd know that at
 * run time and should be able to handle it.
 */
#define __MAX_PROCESSES 1024
#define __MAX_THREADS 1024

#define __MAX_PRIORITY 255

/**
 * The maximum length of the name of a process
 * and thread
 */
#define __MAX_THREAD_NAME_LEN 255
#define __MAX_PROCESS_IMAGE_LEN 255

/**
 * The maximum number of objects allowed
 * in an object table
 */
#define __MAX_OBJECT_TABLE_SIZE 1024

/**
 * The maximum number of shared (named) objects in the registry
 */
#define __MAX_SHARED_OBJECTS (__MAX_OBJECT_TABLE_SIZE * __MAX_PROCESSES)

/**
 * The maximum length of the name of a shared object
 */
#define __MAX_SHARED_OBJECT_NAME_LENGTH 64

/**
 * The stack size for timers TODO replace with argument?
 */
#define __TIMER_STACK_SIZE 1024

/**
 * The maximum number of external interrupt vectors handled
 * by the master/slave interrupt controllers
 */
#define __MAX_ISRS 256

/**
 * The maximum number of alarms the system can support
 */
#define __MAX_ALARMS 1024

#endif /* CONFIG_H_ */
