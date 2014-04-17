/*
 *
 * TINKER Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_IO_H_
#define TGT_IO_H_

#include "tgt_ints.h"

 #define isb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 4" \
                                     : : "r" (0) : "memory")
 #define dsb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" \
                                     : : "r" (0) : "memory")
 #define dmb(x) __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" \
                                     : : "r" (0) : "memory")

#define __out_(N,T) \
	static inline void __out_##N(T * const addr, const T const value) \
	{ \
		*((volatile T *)(addr)) = value; \
		dmb(); \
	}


#define __in_(N,T) \
	static inline T __in_##N(const T * const addr) \
	{ \
		dmb(); \
		return *((volatile T*)(addr)); \
	}

__out_(u8, uint8_t)
__out_(s8, int8_t)
__out_(u16, uint16_t)
__out_(s16, int16_t)
__out_(u32, uint32_t)
__out_(s32, int32_t)
__in_(u8, uint8_t)
__in_(s8, int8_t)
__in_(u16, uint16_t)
__in_(s16, int16_t)
__in_(u32, uint32_t)
__in_(s32, int32_t)

#endif /* TGT_IO_H_ */
