/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "bcm2835_intc.h"

#define IRQ_PENDING_BASIC 0x200
#define IRQ_PENDING_1 0x204
#define IRQ_PENDING_2 0x208
#define IRQ_FIQ_CONTROL 0x20c
#define ENABLE_IRQ_1 0x210
#define ENABLE_IRQ_2 0x214
#define ENABLE_IRQ_BASIC 0x218
#define DISABLE_IRQ_1 0x21c
#define DISABLE_IRQ_2 0x220
#define DISABLE_IRQ_BASIC 0x224

#define MAX_IRQ_REGISTERS 3
#define MAX_IRQS_BASIC 21
#define MAX_IRQS_PER_REQ 32

typedef struct
{
	uint32_t bit;
	uint32_t cause;
} bcm2835_cause_t;

static const bcm2835_cause_t CAUSE_TABLE[MAX_IRQ_REGISTERS][MAX_IRQS_PER_REQ] =
{
		// IRQ Basic
		{
				{1<<0, INTERRUPT_ARM_TIMER},
				{1<<1, INTERRUPT_ARM_MAILBOX},
				{1<<2, INTERRUPT_ARM_DOORBELL_0},
				{1<<3, INTERRUPT_ARM_DOORBELL_1},
				{1<<4, INTERRUPT_VPU0_HALTED},
				{1<<5, INTERRUPT_VPU1_HALTED},
				{1<<6, INTERRUPT_ILLEGAL_TYPE0},
				{1<<7, INTERRUPT_ILLEGAL_TYPE1},
				{1<<8, INTERRUPT_PENDING1},
				{1<<9, INTERRUPT_PENDING2},
				{1<<10, INTERRUPT_JPEG},
				{1<<11, INTERRUPT_USB},
				{1<<12, INTERRUPT_3D},
				{1<<13, INTERRUPT_DMA2},
				{1<<14, INTERRUPT_DMA3},
				{1<<15, INTERRUPT_I2C},
				{1<<16, INTERRUPT_SPI},
				{1<<17, INTERRUPT_I2SPCM},
				{1<<18, INTERRUPT_SDIO},
				{1<<19, INTERRUPT_UART},
				{1<<20, INTERRUPT_ARASANSDIO}
		},
		// IRQ 1
		{
				{1<<0, INTERRUPT_TIMER0},
				{1<<1, INTERRUPT_TIMER1},
				{1<<2, INTERRUPT_TIMER2},
				{1<<3, INTERRUPT_TIMER3},
				{1<<4, INTERRUPT_CODEC0},
				{1<<5, INTERRUPT_CODEC1},
				{1<<6, INTERRUPT_CODEC2},
				{1<<7, INTERRUPT_VC_JPEG},
				{1<<8, INTERRUPT_ISP},
				{1<<9, INTERRUPT_VC_USB},
				{1<<10, INTERRUPT_VC_3D},
				{1<<11, INTERRUPT_TRANSPOSER},
				{1<<12, INTERRUPT_MULTICORESYNC0},
				{1<<13, INTERRUPT_MULTICORESYNC1},
				{1<<14, INTERRUPT_MULTICORESYNC2},
				{1<<15, INTERRUPT_MULTICORESYNC3},
				{1<<16, INTERRUPT_DMA0},
				{1<<17, INTERRUPT_DMA1},
				{1<<18, INTERRUPT_VC_DMA2},
				{1<<19, INTERRUPT_VC_DMA3},
				{1<<20, INTERRUPT_DMA4},
				{1<<21, INTERRUPT_DMA5},
				{1<<22, INTERRUPT_DMA6},
				{1<<23, INTERRUPT_DMA7},
				{1<<24, INTERRUPT_DMA8},
				{1<<25, INTERRUPT_DMA9},
				{1<<26, INTERRUPT_DMA10},
				{1<<27, INTERRUPT_DMA11},
				{1<<28, INTERRUPT_DMA12},
				{1<<29, INTERRUPT_AUX},
				{1<<30, INTERRUPT_ARM},
				{1<<31, INTERRUPT_VPUDMA}
		},
		// IRQ 2
		{
				{1<<0, INTERRUPT_HOSTPORT},
				{1<<1, INTERRUPT_VIDEOSCALER},
				{1<<2, INTERRUPT_CCP2TX},
				{1<<3, INTERRUPT_SDC},
				{1<<4, INTERRUPT_DSI0},
				{1<<5, INTERRUPT_AVE},
				{1<<6, INTERRUPT_CAM0},
				{1<<7, INTERRUPT_CAM1},
				{1<<8, INTERRUPT_HDMI0},
				{1<<9, INTERRUPT_HDMI1},
				{1<<10, INTERRUPT_PIXELVALVE1},
				{1<<11, INTERRUPT_I2CSPISLV},
				{1<<12, INTERRUPT_DSI1},
				{1<<13, INTERRUPT_PWA0},
				{1<<14, INTERRUPT_PWA1},
				{1<<15, INTERRUPT_CPR},
				{1<<16, INTERRUPT_SMI},
				{1<<17, INTERRUPT_GPIO0},
				{1<<18, INTERRUPT_GPIO1},
				{1<<19, INTERRUPT_GPIO2},
				{1<<20, INTERRUPT_GPIO3},
				{1<<21, INTERRUPT_VC_I2C},
				{1<<22, INTERRUPT_VC_SPI},
				{1<<23, INTERRUPT_VC_I2SPCM},
				{1<<24, INTERRUPT_VC_SDIO},
				{1<<25, INTERRUPT_VC_UART},
				{1<<26, INTERRUPT_SLIMBUS},
				{1<<27, INTERRUPT_VEC},
				{1<<28, INTERRUPT_CPG},
				{1<<29, INTERRUPT_RNG},
				{1<<30, INTERRUPT_VC_ARASANSDIO},
				{1<<31, INTERRUPT_AVSPMON}
		}
};

static bool_t bcm2835_get(
		uint32_t * const cause,
		const void * const user_data)
{
#if defined(INTC_DEBUGGING)
	debug_print("BCM2835: determining cause of interrupt\n");
#endif
	bool_t fired = false;
	if (cause && user_data)
	{
		const uint32_t pending_basic = in_u32((uint32_t*)((uint8_t*)user_data + IRQ_PENDING_BASIC));
		uint8_t irq;
		if (pending_basic)
		{
			for (irq = 0 ; irq < MAX_IRQS_BASIC ; irq++)
			{
				if (pending_basic & CAUSE_TABLE[0][irq].bit)
				{
					*cause = CAUSE_TABLE[0][irq].cause;
					fired = true;
					break;
				}
			}
		}
		if (fired && *cause == INTERRUPT_PENDING1)
		{
			const uint32_t pending_1 = in_u32((uint32_t*)((uint8_t*)user_data + IRQ_PENDING_1));
			for (irq = 0 ; irq < MAX_IRQS_PER_REQ ; irq++)
			{
				if (pending_1 & CAUSE_TABLE[1][irq].bit)
				{
					*cause = CAUSE_TABLE[1][irq].cause;
					fired = true;
					break;
				}
			}
		}
		if (fired && *cause == INTERRUPT_PENDING2)
		{
			const uint32_t pending_2 = in_u32((uint32_t*)((uint8_t*)user_data + IRQ_PENDING_2));
			for (irq = 0 ; irq < MAX_IRQS_PER_REQ ; irq++)
			{
				if (pending_2 & CAUSE_TABLE[2][irq].bit)
				{
					*cause = CAUSE_TABLE[2][irq].cause;
					fired = true;
					break;
				}
			}
		}
	}
#if defined(INTC_DEBUGGING)
	debug_print("BCM2835: fired? %d cause=%d\n", fired, *cause);
#endif
	return fired;
}

static void bcm2835_ack(
		const uint32_t cause,
		const void * const user_data)
{
	(void)cause;
	(void)user_data;
	// un-used on the bcm2835 - need to address the cause
}

static void bcm2835_mask(
		const uint32_t cause,
		const void * const user_data)
{
#if defined(INTC_DEBUGGING)
	debug_print("BCM2835: disabling %d\n", cause);
#endif
	if (cause && user_data)
	{
		const uint8_t bank = (uint8_t)(cause/MAX_IRQS_PER_REQ);
		const uint8_t index = (uint8_t)(cause % MAX_IRQS_PER_REQ);
#if defined(INTC_DEBUGGING)
		debug_print("BCM2835: disabling %d bank %d index %d\n", cause, bank, index);
#endif
		uint32_t disable;
		switch (bank)
		{
		case 2:
			disable = in_u32((uint32_t*)((uint8_t*)user_data + DISABLE_IRQ_BASIC));
			disable |= CAUSE_TABLE[0][index].bit;
			out_u32((uint32_t*)((uint8_t*)user_data + DISABLE_IRQ_BASIC), disable);
			break;
		case 0:
			disable = in_u32((uint32_t*)((uint8_t*)user_data + DISABLE_IRQ_1));
			disable |= CAUSE_TABLE[1][index].bit;
			out_u32((uint32_t*)((uint8_t*)user_data + DISABLE_IRQ_1), disable);
			break;
		case 1:
			disable = in_u32((uint32_t*)((uint8_t*)user_data + DISABLE_IRQ_2));
			disable |= CAUSE_TABLE[2][index].bit;
			out_u32((uint32_t*)((uint8_t*)user_data + DISABLE_IRQ_2), disable);
			break;
		default:
			break;
		}
	}
}

static void bcm2835_enable(
		const uint32_t cause,
		const void * const user_data)
{
#if defined(INTC_DEBUGGING)
	debug_print("BCM2835: enabling %d\n", cause);
#endif
	if (cause && user_data)
	{
		const uint8_t bank = (uint8_t)(cause/MAX_IRQS_PER_REQ);
		const uint8_t index = (uint8_t)(cause % MAX_IRQS_PER_REQ);
#if defined(INTC_DEBUGGING)
		debug_print("BCM2835: enabling %d bank %d index %d\n", cause, bank, index);
#endif
		//uint32_t enabled;
		uint16_t offset;
		uint32_t bit;
		switch (bank)
		{
		case 2:
			offset = ENABLE_IRQ_BASIC;
			bit = CAUSE_TABLE[0][index].bit;
			break;
		case 0:
			offset = ENABLE_IRQ_1;
			bit = CAUSE_TABLE[1][index].bit;
			break;
		case 1:
			offset = ENABLE_IRQ_2;
			bit = CAUSE_TABLE[2][index].bit;
			break;
		default:
			break;
		}
		const uint32_t enabled = in_u32((uint32_t*)((uint8_t*)user_data + offset));
#if defined(INTC_DEBUGGING)
		debug_print("BCM2835: enabling base %x, offset %x, enabled %x -> %x\n", user_data, offset, enabled, enabled | bit);
#endif
		out_u32((uint32_t*)((uint8_t*)user_data + offset), enabled | bit);
	}
}

static void bcm2835_setup(
		const uint32_t cause,
		const intc_priority_t priority,
		const intc_detection_type detection,
		const intc_active_type edge_type,
		const void * const user_data)
{
	(void)cause;
	(void)priority;
	(void)detection;
	(void)edge_type;
	(void)user_data;
	// unused - simple controller
}


intc_t* bcm2835_intc_create(
		mem_pool_info_t * const pool,
		const uint8_t * const base_address)
{
	intc_t * intc = NULL;
	kernel_intc_t * const intc_device = (kernel_intc_t*)mem_alloc(pool, sizeof(kernel_intc_t));
	if (intc_device)
	{
		intc_device->get_cause = bcm2835_get;
		intc_device->ack_cause = bcm2835_ack;
		intc_device->enable_cause = bcm2835_enable;
		intc_device->mask_cause = bcm2835_mask;
		intc_device->setup_cause = bcm2835_setup;
		intc_device->user_data = base_address;
		intc = intc_create(pool, intc_device);
	}
	else
	{
		intc = NULL;
	}
	return intc;
}
