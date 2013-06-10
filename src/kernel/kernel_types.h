/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
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

#ifndef KERNEL_TYPES_H_
#define KERNEL_TYPES_H_

#include "config.h"
#include "arch/target_types.h"
#include "api/sos3_api_types.h"
#include "api/sos3_api_errors.h"

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

typedef void (__timer_setup)(void * usr_data,__time_t timeout, __timer_callback * call_back);

typedef void (__timer_cancel)(void * usr_data);

typedef struct
{
	__timer_setup *		timer_setup;
	__timer_cancel *	timer_cancel;
	void * 				usr_data;
	uint32_t 				usr_data_size;
} __timer_t;

typedef void(__alarm_call_back)(
		void * usr_data,
		uint32_t usr_data_param);

/*
 * ========================================
 * == Object Types			             ==
 * ========================================
 */

typedef uint8_t priority_t;

typedef enum __object_type
{
	UNKNOWN_OBJ = 0,
	OBJECT = 1,
	PROCESS_OBJ = 2,
	THREAD_OBJ = 3,
	PIPE_OBJ = 4,
	SEMAPHORE_OBJ = 5,
	SHARED_MEMORY_OBJ = 6,
	CLOCK_OBJ = 7,
	TIMER_OBJ = 8
} __object_type_t;

typedef enum __object_pipe_direction
{
	PIPE_DIRECTION_UNKNOWN,
	PIPE_BI_DIRECTIONAL,
	PIPE_DIRECTIONAL
} __object_pipe_direction_t;

typedef struct __object
{
	object_number_t object_number;
	uint16_t initialised;
	__spinlock_t lock;
	uint16_t allocated;
	__object_type_t type;
	uint32_t ref_count;

	union
	{
		struct
		{
			uint32_t pid;
		} process;

		struct
		{
			uint32_t pid;
			uint32_t tid;
			struct __thread_ * thread;
			/**
			 * This field is used to store the original priority
			 * of a thread during the use-case of priority inheritance
			 */
			priority_t priority_inheritance;
			priority_t original_priority;
		} thread;

		struct
		{
			uint32_t pid_from;
			uint32_t pid_to;
			__object_pipe_direction_t direction;
		} pipe;

		struct
		{
			uint32_t sem_count;
			uint32_t sem_alloc;
			void * listeners;
			void * owners;
			priority_t highest_priority;
		} semaphore;

		struct
		{
			uint32_t real_start;
			uint32_t virtual_start;
			uint32_t size;
		} shared_memory;

		struct
		{
			__time_t time;
		} clock;

		struct
		{
			__time_t timeout;
		} timer;

	} specifics;
} __object_t;

typedef struct
{
	void* the_map;
	uint32_t next_id;
} __object_table_t;

/*
 * ========================================
 * == Process Types			             ==
 * ========================================
 */
typedef void(__thread_entry)(void);

typedef struct __thread_
{
	uint32_t			thread_id;
	void 				* stack;
	/** The base address of the stack in real memory */
	uint32_t		    v_stack_base;
	uint32_t			r_stack_base;
	uint32_t			stack_size;
	priority_t			priority;
	uint32_t			flags;
	struct __process *	parent;
	__thread_state_t	state;
	__object_t *		waiting_on;
	__thread_entry * 	entry_point;
	object_number_t		object_number;
	/*
	 * TODO May need other entries for data/bss after
	 * we've written the ELF importer
	 */
	uint8_t				context[__MAX_CONTEXT_SIZE];
	char 				name[__MAX_THREAD_NAME_LEN + 1];
} __thread_t;

typedef struct __process
{
	uint32_t				process_id;
	void *					threads;
	uint32_t				thread_count;
	__mem_pool_info_t * 	memory_pool;
	__object_table_t		object_table;
	object_number_t			object_number;
	bool					kernel_process;
	segment_info_t			segment_info;
	mmu_section_t *			first_section;
	char					image[__MAX_PROCESS_IMAGE_LEN + 1];
} __process_t;

/*
 * ========================================
 * == Scheduler Types                ==
 * ========================================
 */

typedef void(__sch_initialise_t)(void);
typedef void(__sch_implementation_t)(__thread_t ** new_thread);
typedef __thread_t*(__sch_get_curr_thread_t)(void);
typedef void(__sch_set_curr_thread_t)(__thread_t* t);
typedef void(__sch_notify_new_thread_t)(__thread_t * t);
typedef void(__sch_notify_exit_thread_t)(__thread_t * t);
typedef void(__sch_notify_pause_thread_t)(__thread_t * t);
typedef void(__sch_notify_resume_thread_t)(__thread_t * t);
typedef void(__sch_notify_change_priority_t)(__thread_t * t, priority_t original_priority);

typedef struct __scheduler
{
	__sch_initialise_t * 			initialise;
	__sch_implementation_t*  		scheduler;
	__sch_get_curr_thread_t* 		get_curr_thread;
	__sch_set_curr_thread_t*		set_curr_thread;
	__sch_notify_new_thread_t*		new_thread;
	__sch_notify_exit_thread_t* 	exit_thread;
	__sch_notify_pause_thread_t*	pause_thread;
	__sch_notify_resume_thread_t*	resume_thread;
	__sch_notify_change_priority_t*	change_priority;
} __scheduler_t;

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

typedef error_t (__kernel_device_control)(void * usr_data, uint32_t code);

typedef error_t (__kernel_device_write_register)(void * usr_data, uint32_t id, uint32_t val);

typedef error_t (__kernel_device_read_register)(void * usr_data, uint32_t id, uint32_t * val);

typedef error_t (__kernel_device_write_buffer)(void * usr_data, const uint32_t dst, void * src, const uint32_t src_size);

typedef error_t (__kernel_device_read_buffer)(void * usr_data, const uint32_t src, void * dst, const uint32_t dst_size);

typedef error_t (__kernel_device_isr)(void * usr_data, uint32_t vector);

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
	void * user_data;
} __kernel_device_t;

#endif /* KERNEL_TYPES_H_ */
