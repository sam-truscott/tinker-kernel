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

#define __out_(N,T) \
	static inline void __out_##N(T * const addr, const T const value) \
	{ \
		*((volatile T *)(addr)) = value; \
		__asm__("eieio"); \
	}


#define __in_(N,T) \
	static inline T __in_##N(const T * const addr) \
	{ \
		__asm__("eieio"); \
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
