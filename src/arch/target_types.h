/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
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

#define SWAP_32(V) \
	(V & 0x000000FF) << 24 | \
	(V & 0x0000FF00) << 8  | \
	(V & 0x00FF0000) >> 8  | \
	(V & 0xFF000000) >> 24

#endif /* TARGET_TYPES_H_ */
