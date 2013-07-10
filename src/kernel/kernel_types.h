/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef KERNEL_TYPES_H_
#define KERNEL_TYPES_H_

#include "config.h"
#include "arch/target_types.h"
#include "api/sos_api_types.h"
#include "api/sos_api_errors.h"

/*
 * ========================================
 * == Memory Management Types            ==
 * ========================================
 */

/*
 * this type is used to track a single block
 * (aka allocation) on the heap
 */
#pragma pack(push, 1)
typedef struct __mem_block
{
	uint32_t addr;
	uint32_t size;
	uint16_t offset;
	bool is_free;
} __mem_block_t;
#pragma pack(pop)

/*
 * this structure is used to manage the blocks
 * stored on the heap
 */
#pragma pack(push, 1)
typedef struct __mem_pool_info
{
	/* address of the start of the heap */
	uint32_t start_pool;
	/* address of the end of the heap */
	uint32_t end_pool;
	/* keep track of the heap size in bytes */
	uint32_t	free_pool_bytes;
	/* the number of blocks allocated */
	uint32_t blocks;
	/* location of the first heap info block.
	 * this should go backwards in memory from
	 * here to the last block */
	__mem_block_t *	first_block;
	/* the size of a single page */
	uint32_t page_size;
	/* total heap in pages */
	uint32_t total_pages;
	/* total heap in bytes */
	uint32_t total_pool_size;
	/* if this block becomes full this is
	 * the next one to use */
	struct __mem_pool_info * next_pool_info;
} __mem_pool_info_t;
#pragma pack(pop)

#define MMU_NO_ACCESS	0
#define MMU_R_ACCESS	1
#define MMU_W_ACCESS	2
#define MMU_E_ACCESS	4

typedef enum
{
	mmu_device_memory,
	mmu_random_access_memory
} mmu_memory_t;

typedef enum
{
	mmu_no_privilege,
	mmu_user_access,
	mmu_kernel_access,
	mmu_all_access
} mmu_privilege_t;

typedef enum
{
	mmu_no_access	= 0,
	mmu_read_only	= 1,
	mmu_read_write	= 2
} mmu_access_t;

typedef uint32_t seg_id_t;

#define MAX_MMU_NAME_LEN 32u

#define EMPTY_MMU_SECTON_NAME ((mmu_section_name_t){'\0'})

typedef struct mmu_section
{
	struct mmu_section *prev;
	struct mmu_section *next;
	uint32_t 			real_address;
	uint32_t			virt_address;
	uint32_t 			size;
	mmu_memory_t		memory_type;
	mmu_privilege_t		privilege;
	mmu_access_t		access_rights;
} mmu_section_t;

typedef struct
{
	uint32_t segment_ids[MMU_SEG_COUNT];
} segment_info_t;

/*
 * ========================================
 * == Clock Types                        ==
 * ========================================
 */

#define ONE_SECOND_AS_NANOSECONDS	1000000000
#define ONE_MS_AS_NANOSECONDS 		1000000
#define ONE_US_AS_NANOSECONDS 		1000

typedef struct
{
	int32_t seconds;
	int64_t nanoseconds;
} __time_t;

#define __ZERO_TIME ((__time_t){0,0})

typedef uint64_t (__clock_get_nanoseconds_time)(void);

typedef struct
{
	__clock_get_nanoseconds_time * 		get_time;
} __clock_device_t;

typedef void (__timer_callback)(void);

typedef void (__timer_setup)(
		const void * const usr_data,
		const __time_t timeout,
		__timer_callback * const call_back);

typedef void (__timer_cancel)(
		const void * const usr_data);

typedef struct
{
	__timer_setup *		timer_setup;
	__timer_cancel *	timer_cancel;
	void * 				usr_data;
	uint32_t 			usr_data_size;
} __timer_t;

typedef void(__alarm_call_back)(
		const void * const usr_data,
		const uint32_t usr_data_param);

/*
 * ========================================
 * == Device Driver Types                ==
 * ========================================
 */

struct __kernel_device;

typedef struct __kernel_device_info
{
	uint8_t	major_version;
	uint8_t	minor_version;
	uint8_t	revision;
	uint32_t	build;
}__kernel_device_info_t;

typedef error_t (__kernel_device_initialise)(
		struct __kernel_device * device_info,
		void * param,
		const uint32_t param_size);

typedef error_t (__kernel_device_control)(
		void * usr_data, uint32_t code);

typedef error_t (__kernel_device_write_register)(
		const void * const usr_data,
		const uint32_t id,
		const uint32_t val);

typedef error_t (__kernel_device_read_register)(
		const void * const usr_data, const uint32_t id, uint32_t * const val);

typedef error_t (__kernel_device_write_buffer)(
		const void * const usr_data,
		const uint32_t dst,
		const void * const src,
		const uint32_t src_size);

typedef error_t (__kernel_device_read_buffer)(
		const void * const usr_data,
		const uint32_t src,
		void * const dst,
		const uint32_t dst_size);

typedef error_t (__kernel_device_isr)(
		const void * const usr_data,
		const uint32_t vector);

typedef struct __kernel_device
{
	__kernel_device_info_t		 	info;
	__kernel_device_initialise * 	initialise;
	__kernel_device_control	*		control;
	__kernel_device_write_register *write_register;
	__kernel_device_read_register *	read_register;
	__kernel_device_write_buffer *	write_buffer;
	__kernel_device_read_buffer *	read_buffer;
	__kernel_device_isr * 			isr;
	const void * user_data;
} __kernel_device_t;

#endif /* KERNEL_TYPES_H_ */
