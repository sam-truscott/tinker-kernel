/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef KERNEL_LOCKS_LOCK_H_
#define KERNEL_LOCKS_LOCK_H_

typedef unsigned int lock_t;

void lock_init(lock_t * const lock);

void lock(lock_t * const lock);

void unlock(lock_t * const lock);

#endif /* KERNEL_LOCKS_LOCK_H_ */
