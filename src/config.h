/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

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
 * The maximum size of the interrupt vector context
 */
#define __MAX_CONTEXT_SIZE 168 + 256 /* Based on PPC32 - 256 for FP Registers */

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
 * The maximum number of hardware devices in the system
 */
#define __MAX_DEVICES 1024

/**
 * The maximum number of external interrupt vectors handled
 * by the master/slave interrupt controllers
 */
#define __MAX_ISRS 1024

/**
 * The maximum number of alarms the system can support
 */
#define __MAX_ALARMS 1024

#endif /* CONFIG_H_ */
