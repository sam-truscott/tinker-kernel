/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

#include "arch/tgt_types.h"
#include "x86_registers.h"
#include "kernel/console/print_out.h"

#define MAX_GDT_ENTRIES 6u

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04u) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07u) // Present
#define SEG_SAVL(x)      ((x) << 0x0Cu) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0Du) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0Eu) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0Fu) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03u) << 0x05u)   // Set privilege level (0 - 3)

#define SEG_DATA_RD        0x00u // Read-Only
#define SEG_DATA_RDA       0x01u // Read-Only, accessed
#define SEG_DATA_RDWR      0x02u // Read/Write
#define SEG_DATA_RDWRA     0x03u // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04u // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05u // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06u // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07u // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08u // Execute-Only
#define SEG_CODE_EXA       0x09u // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0Au // Execute/Read
#define SEG_CODE_EXRDA     0x0Bu // Execute/Read, accessed
#define SEG_CODE_EXC       0x0Cu // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0Du // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0Eu // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0Fu // Execute/Read, conforming, accessed

#define GDT_CODE_PL0 SEG_DESCTYPE(1u) | SEG_PRES(1u) | SEG_SAVL(0u) | \
                     SEG_LONG(0u)     | SEG_SIZE(1u) | SEG_GRAN(1u) | \
                     SEG_PRIV(0u)     | SEG_CODE_EXRD

#define GDT_DATA_PL0 SEG_DESCTYPE(1u) | SEG_PRES(1u) | SEG_SAVL(0u) | \
                     SEG_LONG(0u)     | SEG_SIZE(1u) | SEG_GRAN(1u) | \
                     SEG_PRIV(0u)     | SEG_DATA_RDWR

#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1u) | SEG_SAVL(0u) | \
                     SEG_LONG(0u)     | SEG_SIZE(1u) | SEG_GRAN(1u) | \
                     SEG_PRIV(3u)     | SEG_CODE_EXRD

#define GDT_DATA_PL3 SEG_DESCTYPE(1u) | SEG_PRES(1u) | SEG_SAVL(0u) | \
                     SEG_LONG(0u)     | SEG_SIZE(1u) | SEG_GRAN(1u) | \
                     SEG_PRIV(3u)     | SEG_DATA_RDWR

typedef struct gdt
{
	uint32_t base;
	uint32_t limit;
	uint16_t flags;
} gdt_t;

struct gdt_ptr
{
	uint16_t len;
	uint32_t ptr;
} __attribute__((packed));

static uint32_t x86_tss  __attribute__((aligned(16)));

static const gdt_t x86_gdt_data[MAX_GDT_ENTRIES] =
{
	{0u, 0u, 0u},
	{0u, 0xFFFFFFFFu, GDT_CODE_PL0},
	{0u, 0xFFFFFFFFu, GDT_DATA_PL0},
	{0u, 0xFFFFFFFFu, GDT_CODE_PL3},
	{0u, 0xFFFFFFFFu, GDT_DATA_PL3},
	{(uint32_t)&x86_tss, sizeof(x86_tss), GDT_DATA_PL0}
};
static uint64_t x86_gdt_table[MAX_GDT_ENTRIES] __attribute__((aligned(16)));

static void x86_dump_cr0(const uint32_t cr0);
static void x86_create_gdt_entry(uint8_t index, const gdt_t * const gdt_entry);

static inline uint16_t ds(void)
{
	uint16_t seg;
	asm("movw %%ds,%0" : "=rm" (seg));
	return seg;
}

void x86_initialise_gdt(void)
{
	const uint32_t cr0 = x86_get_cr0();
	x86_dump_cr0(cr0);
	if (!(cr0 & 1))
	{
		debug_print("Setting up GDT\n");
		for (uint8_t e = 0 ; e < MAX_GDT_ENTRIES ; e++)
		{
			debug_print("GDT %d: Base=%x, Limit=%x, Type=%x\n",
					e,
					x86_gdt_data[e].base,
					x86_gdt_data[e].limit,
					x86_gdt_data[e].flags);
			x86_create_gdt_entry(e, &(x86_gdt_data[e]));
		}
		debug_print("GDT Table build\n");
		static struct gdt_ptr gdt;
		gdt.len = sizeof(x86_gdt_table)-1;
		gdt.ptr = (uint32_t)&x86_gdt_table + (ds() << 4);
		asm volatile("lgdtl %0" : : "m" (gdt));
		printp_out("GDT Table loaded\n");
	}
}

static void x86_create_gdt_entry(const uint8_t index, const gdt_t * const gdt_entry)
{
    // Create the high 32 bit segment
	x86_gdt_table[index]  =  gdt_entry->limit       & 0x000F0000;         // set limit bits 19:16
	x86_gdt_table[index] |= (gdt_entry->flags <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    x86_gdt_table[index] |= (gdt_entry->base >> 16) & 0x000000FF;         // set base bits 23:16
    x86_gdt_table[index] |=  gdt_entry->base        & 0xFF000000;         // set base bits 31:24

    // Shift by 32 to allow for low part of segment
    x86_gdt_table[index] <<= 32;

    // Create the low 32 bit segment
    x86_gdt_table[index] |= gdt_entry->base  << 16;                       // set base bits 15:0
    x86_gdt_table[index] |= gdt_entry->limit  & 0x0000FFFF;               // set limit bits 15:0
}

static void x86_dump_cr0(const uint32_t cr0)
{
	uint32_t x = 90000000;
	while(x--) {}
	debug_print("CR0=%d\n", cr0);
	x = 90000000;
	while(x--) {}
	if (cr0 & 1)
	{
		debug_print("CR0: Protected Mode\n");
	}
	else
	{
		debug_print("CR0: Real Mode\n");
	}
	if (cr0 & 2)
	{
		debug_print("CR0: Multi-processor system\n");
	}
	else
	{
		debug_print("CR0: Uniprocessor system\n");
	}
	if (cr0 & 4)
	{
		debug_print("CR0: No floating point unit present\n");
	}
	else
	{
		debug_print("CR0: Floating point unit present\n");
	}
}
