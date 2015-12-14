/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef TGT_IO_H_
#define TGT_IO_H_

#include "tgt_types.h"

#pragma GCC optimize ("-O0")
#define out_(N,T) \
	static inline void out_##N(T * const addr, T value) \
	{ \
		*((volatile T *)(addr)) = value; \
		asm volatile ("DMB"); \
	}

#define in_(N,T) \
	static inline T in_##N(const T * const addr) \
	{ \
		asm volatile ("DMB"); \
		return *((volatile T*)(addr)); \
	}

out_(u8, uint8_t)
out_(s8, int8_t)
out_(u16, uint16_t)
out_(s16, int16_t)
out_(u32, uint32_t)
out_(s32, int32_t)
in_(u8, uint8_t)
in_(s8, int8_t)
in_(u16, uint16_t)
in_(s16, int16_t)
in_(u32, uint32_t)
in_(s32, int32_t)

#endif /* TGT_IO_H_ */
