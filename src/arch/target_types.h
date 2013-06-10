/*************************************************************************
 *
 * SOS 3 Source Code
 * __________________
 *
 *  [2012] Samuel Steven Truscott
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

#ifndef TARGET_TYPES_H_
#define TARGET_TYPES_H_

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;

typedef volatile uint8_t __spinlock_t;
#define LOCK_ON 0xDE
#define LOCK_OFF 0xED

#define MMU_PAGE_SIZE 	4096
#define MMU_SEG_SIZE 	((256 * 1024) * 1024)
#define MMU_SEG_COUNT	16
#define MMU_MAX_SIZE	(MMU_SEG_SIZE * MMU_SEG_COUNT)

#endif /* TARGET_TYPES_H_ */
