/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#ifndef OPIC_PRIVATE_H_
#define OPIC_PRIVATE_H_

#define OPIC_BIG_ENDIAN

/**
 * The register region base addresses
 */
typedef enum
{
	OPIC_ISU_BASE = 0x10000,
	OPIC_PROC_BASE = 0x20000,
	OPIC_TIMER_BASE = 0x01100
} opic_reg_block_t;

/**
 * ISU registers
 */
typedef enum
{
	INTC_SRC_N_VECT_PRIORITY_REGISTER = 0x00,
	INTC_SRC_N_DEST_REGISTER = 0x10
} opic_isu_reg_t;

/**
 * Bits of the Vector priority register
 */
typedef enum {
  ISU_MASK_BIT =  (1 << 31),
  ISU_ACTIVE_BIT = 0x40000000,
  ISU_MULTICAST_BIT = 0x20000000,
  ISU_POSITIVE_POLARITY_BIT = 0x00800000,
  ISU_LEVEL_TRIGGERED_BIT = 0x00400000,
} opic_isu_vect_pri_reg_t;

#define ISU_SHIFT_PRIORITY 16
#define ISU_SHIFT_VECTOR 0
#define ISU_BLOCK_SIZE 32

/**
 * Proc registers
 */
typedef enum
{
	N_DISPATCH_REGISTER_1 = 0x40,
	N_DISPATCH_REGISTER_2 = 0x50,
	N_DISPATCH_REGISTER_3 = 0x60,
	N_DISPATCH_REGISTER_4 = 0x70,
	TASK_PRIORITY_REGISTER_N = 0x80,
	INTERRUPT_ACK_REGISTER_N = 0xA0,
	END_OF_INTERRUPT_REGISTER_N = 0xB0
} opic_proc_reg_t;

/**
 * Timer registers
 */
typedef enum
{
	TMR_N_CURRENT_COUNT_REGISTER = 0x00,
	TMR_N_BASE_COUNT_REGISTER = 0x10,
	TMR_N_VECTOR_PRIORITY_REGISTER = 0x20,
	TMR_N_DEST_REGISTER = 0x30
} opic_timer_reg_t;

/**
 * Misc. registers
 */
typedef enum
{
	TMR_FREQ_REPORT_REGISTER = 0x10F0,
	SPURIOUS_VECT_REGISTER = 0x10E0,
	N_VECT_PRIORITY_REGISTER = 0x10A0,
	PROC_INIT_REGISTER = 0x1090,
	VENDOR_IDENT_REGISTER = 0x1080,
	GLOBAL_CONF_REGISTER = 0x1020,
	FEATURE_REPORTING_REGISTER = 0x1000
} opic_misc_reg_t;

#endif /* OPIC_PRIVATE_H_ */
