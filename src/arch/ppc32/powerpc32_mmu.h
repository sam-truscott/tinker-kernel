/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef POWERPC32_7400_MMU_H_
#define POWERPC32_7400_MMU_H_

#include "arch/tgt_types.h"

/*
 * Operations for the 7400 MMU
 */

typedef enum __ppc32_bl
{
        __ppc32_kb_128        = 0,
        __ppc32_kb_256        = 1,
        __ppc32_kb_512        = 3,
        __ppc32_mb_1        = 7,
        __ppc32_mb_2        = 15,
        __ppc32_mb_4        = 31,
        __ppc32_mb_8        = 63,
        __ppc32_mb_16        = 127,
        __ppc32_mb_32        = 255,
        __ppc32_mb_64        = 511,
        __ppc32_mb_128        = 1023,
        __ppc32_mb_256        = 2047
} __ppc32_bl_t;

typedef enum __ppc32_pp
{
        __ppc32_no_access        = 0,
        __ppc32_read_only        = 1,
        __ppc32_read_write        = 2
} __ppc32_pp_t;


#define __ppc32_no_user_access			0
#define __ppc32_user_access				1
#define __ppc32_no_supervisor_access	0
#define __ppc32_supervisor_access		1

/**
 * If write-back the update is written to cache and only
 * to the device just before it is replaced.
 */
#define __ppc32_write_back			0x0
/**
 * If write-through is enabled the data is written to the
 * cache and the backing store (device).
 */
#define __ppc32_write_through		0x8

/**
 * If cache is inhibited it will always access the
 * backing store directly and not access the cache.
 */
#define __ppc32_cache_inhibited 	0x4
#define __ppc32_cache_enabled		0x0

/**
 * Used in multiple-processor designs
 */
#define __ppc32_memory_coherency	0x2
#define __ppc32_memory_no_coherency	0

/**
 * Guarding prohibits speculative (out-of-order) accesses
 * to devices
 */
#define __ppc32_guarded				0x1
#define __ppc32_not_guarded			0x0

#define __PPC32_BEPI_BRPN(X) \
	((X) >> 17u)

#define __PPC32_WIMG(W, I, M, G) \
	W | I | M | G

/**
 * Macro to build the Upper BAT Register value
 */
#define __PPC32_BATU(BEPI, BL, VS, VP) \
	(BEPI << 17) \
	| (BL << 2) \
	| (VS << 1) \
	| (VP)

/**
 * Macro to build the Lower BAT register value
 */
#define __PPC32_BATL(BRPN, WIMG, PP) \
	(BRPN << 17) \
	| (WIMG << 3) \
	| (PP)

/**
 * Supervisor (Kernel) FAIL
 */
#define SR_KS_FAIL	0u
/**
 * Supervisor (Kernel) OK
 */
#define SR_KS_OK	1u
/**
 * Problem (User) Fail
 */
#define	SR_KP_FAIL	0u
/**
 * Problem (User) Ok
 */
#define SR_KP_OK	1u

#define SR_NE_OFF	0u
#define SR_NE_ON	1u

/**
 * Ks = Supervisor protection key,
 * Kp = Problem/User protection key,
 * N = No-execute protection,
 * Virtual Segment ID
 */
#define __PPC_SR_T0(Ks, Kp, N, VSID) \
	((uint32_t)(Ks) << 30) \
	| ((uint32_t)(Kp) << 29) \
	| ((uint32_t)(N) << 28) \
	| ((uint32_t)(VSID))

/**
 * Ks = Supervisor protection key,
 * Kp = Problem/User protection key,
 * BUID = Bus Unit ID,
 * CSI - Controller-Specific Information
 */
#define __PPC_SR_T1(Ks, Kp, BUID, CSI) \
	(1 << 31) \
	| (Ks << 30) \
	| (Kp << 29) \
	| (BUID << 21) \
	(CSI)

#define PTE_VALID	1u
#define PTE_INVALUD	0u

#define HASH_PRIMARY 	0u
#define HASH_SECONDARY 	1u

#define __PPC_PTE_W0(V, VSID, H, API) \
	(V << 31) \
	| (VSID << 7) \
	| (H << 6) \
	| (API & 0x3Fu)

#define __PPC_PTE_W1(RPN, R, C, WIMG, PP) \
	( RPN & 0xFFFFF000u ) \
	| (R << 8) /* Referenced / Accessed */ \
	| (C << 7) /* Changed / Dirty */ \
	| (WIMG << 3) \
	| PP

#define __PPC_GET_SEGMENT_INDEX(ea) \
	((((uint32_t)ea) >> 28) & 0xFu)

/**
 * Invalid the data and instruction MMUs (tlbie).
 * This will perform a sync prior to the invalidation and
 * also a tlbsync afterwards to ensure that it's completed.
 * @param ie The TLB Entnry to invalidate.
 * Must be between 0 and MAX_D/ITLB_ENTRIES - 1
 */
void __ppc_invalid_tlbe(uint32_t ie);

/*
 * Block Address Translation (BAT) registers
 */
void __ppc32_set_ibat0l(uint32_t bat);
void __ppc32_set_ibat0u(uint32_t bat);
void __ppc32_set_ibat1l(uint32_t bat);
void __ppc32_set_ibat1u(uint32_t bat);
void __ppc32_set_ibat2l(uint32_t bat);
void __ppc32_set_ibat2u(uint32_t bat);
void __ppc32_set_ibat3l(uint32_t bat);
void __ppc32_set_ibat3u(uint32_t bat);

void __ppc32_set_dbat0l(uint32_t bat);
void __ppc32_set_dbat0u(uint32_t bat);
void __ppc32_set_dbat1l(uint32_t bat);
void __ppc32_set_dbat1u(uint32_t bat);
void __ppc32_set_dbat2l(uint32_t bat);
void __ppc32_set_dbat2u(uint32_t bat);
void __ppc32_set_dbat3l(uint32_t bat);
void __ppc32_set_dbat3u(uint32_t bat);

/*
 * Segment Registers (SR) - each segment is 256Mb of 4GB
 * giving a total of 16 segment registers for the 32bit
 * memory space
 */

void __ppc32_set_sr0(uint32_t sr);
void __ppc32_set_sr1(uint32_t sr);
void __ppc32_set_sr2(uint32_t sr);
void __ppc32_set_sr3(uint32_t sr);
void __ppc32_set_sr4(uint32_t sr);
void __ppc32_set_sr5(uint32_t sr);
void __ppc32_set_sr6(uint32_t sr);
void __ppc32_set_sr7(uint32_t sr);
void __ppc32_set_sr8(uint32_t sr);
void __ppc32_set_sr9(uint32_t sr);
void __ppc32_set_sr10(uint32_t sr);
void __ppc32_set_sr10(uint32_t sr);
void __ppc32_set_sr11(uint32_t sr);
void __ppc32_set_sr12(uint32_t sr);
void __ppc32_set_sr13(uint32_t sr);
void __ppc32_set_sr14(uint32_t sr);
void __ppc32_set_sr15(uint32_t sr);

uint32_t __ppc32_get_sr0(void);
uint32_t __ppc32_get_sr1(void);
uint32_t __ppc32_get_sr2(void);
uint32_t __ppc32_get_sr3(void);
uint32_t __ppc32_get_sr4(void);
uint32_t __ppc32_get_sr5(void);
uint32_t __ppc32_get_sr6(void);
uint32_t __ppc32_get_sr7(void);
uint32_t __ppc32_get_sr8(void);
uint32_t __ppc32_get_sr9(void);
uint32_t __ppc32_get_sr10(void);
uint32_t __ppc32_get_sr10(void);
uint32_t __ppc32_get_sr11(void);
uint32_t __ppc32_get_sr12(void);
uint32_t __ppc32_get_sr13(void);
uint32_t __ppc32_get_sr14(void);
uint32_t __ppc32_get_sr15(void);

/*
 * The SDR contains the base address and size
 * of the page tables in memory
 */

/**
 * HTABORG - Physical Address of the Page Table in RAM
 * HTABMASK - Mask for page table address
 */
#define __PPC_SDR(HTABORG, HTABMASK) \
		( ((uint32_t)HTABORG) & 0xFFFF0000u) \
		| (uint32_t)HTABMASK

/**
 * Macro to generate the primary HASH
 */
#define __PPC_PRIMARY_HASH(VSID, PAGE_INDEX) \
	(VSID & 0x7FFFF) ^ PAGE_INDEX;

#define __PPC_PRIMARY_HASH_EA(VSID, EA) \
		__PPC_PRIMARY_HASH(VSID, ((EA >> 19) & 0xFFFF))

#define __PPC_PRIMARY_HASH_VA(VSID, VA) \
		__PPC_PRIMARY_HASH(VSID, ((EA >> 39) & 0xFFFF))

void __ppc32_set_sdr1(const uint32_t sdr1);

tgt_pg_tbl_t __ppc32_get_sdr1(void);

void __ppc32_add_pte(
		const tgt_pg_tbl_t * const page_tbl,
		const uint32_t ea,
		const uint32_t vsid,
		const uint32_t pte_w0,
		const uint32_t pte_w1);

void __ppc32_remove_pte(
		const tgt_pg_tbl_t * const page_tbl,
		const uint32_t ea,
		const uint32_t vsid,
		const uint32_t pte_w0,
		const uint32_t pte_w1);

void __ppc32_switch_page_table(
        const __process_t * const last_proc,
        const __process_t * const proc);

#endif /* POWERPC32_7400_MMU_H_ */
