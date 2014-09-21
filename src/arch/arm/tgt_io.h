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

#include "arch/tgt_types.h"

#define out_(N,T) \
	static inline void out_##N(T * const addr, T value) \
	{ \
		*((volatile T *)(addr)) = value; \
		asm volatile ("mcr	p15, 0, r0, c7, c10, 4"); \
	}

#define in_(N,T) \
	static inline T in_##N(const T * const addr) \
	{ \
		asm volatile ("mcr	p15, 0, r0, c7, c10, 4"); \
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
