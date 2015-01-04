/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef ARCH_ARM_ARM_MMU_H_
#define ARCH_ARM_ARM_MMU_H_

void arm_invalidate_all_tlbs(void);

void arm_disable_mmu(void);

void arm_enable_mmu(void);

#endif /* ARCH_ARM_ARM_MMU_H_ */
