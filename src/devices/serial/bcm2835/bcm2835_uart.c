/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2014] Samuel Steven Truscott
 *  All Rights Reserved.
 */

/* uart.c - UART initialization & communication */
/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 13: UART
 */

#include "arch/tgt_types.h"
#include "tgt_io.h"
#include "bcm2835_uart.h"
#include "kernel/kernel_in.h"

// The GPIO registers base address.
#define GPIO_BASE 0x20200000

// The offsets for reach register.
// Controls actuation of pull up/down to ALL GPIO pins.
#define GPPUD  (GPIO_BASE + 0x94)

// Controls actuation of pull up/down for specific GPIO pin.
#define GPPUDCLK0  (GPIO_BASE + 0x98)

// The base address for UART.
#define UART0_BASE  0x20201000

// The offsets for reach register for the UART.
#define UART0_DR      0x00
#define UART0_RSRECR  0x04
#define UART0_FR      0x18
#define UART0_ILPR    0x20
#define UART0_IBRD    0x24
#define UART0_FBRD    0x28
#define UART0_LCRH    0x2C
#define UART0_CR      0x30
#define UART0_IFLS    0x34
#define UART0_IMSC    0x38
#define UART0_RIS     0x3C
#define UART0_MIS     0x40
#define UART0_ICR     0x44
#define UART0_DMACR   0x48
#define UART0_ITCR    0x80
#define UART0_ITIP    0x84
#define UART0_ITOP    0x88
#define UART0_TDR     0x8C

/*
 * delay function
 * int32_t delay: number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * wont optimize away.
 */
static void delay(const uint32_t count) {
    asm volatile(
    		"__delay_%=:"
    		"subs %[count],"
    		"%[count], #1; bne __delay_%=\n"
	     : : [count]"r"(count) : "cc");
}

/*
 * Initialize UART0.
 */
void early_uart_init() {
    // Disable UART0.
    out_u32((uint32_t*)(UART0_BASE + UART0_CR), 0x00000000);
    delay(200);
    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    out_u32((uint32_t*)GPPUD, 0x00000000);
    delay(200);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    out_u32((uint32_t*)GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(200);

    // Write 0 to GPPUDCLK0 to make it take effect.
    out_u32((uint32_t*)GPPUDCLK0, 0x00000000);

    // Clear pending interrupts.
    out_u32((uint32_t*)(UART0_BASE + UART0_ICR), 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    // Divider = 3000000/(16 * 9600) = 19
    // Fractional part register = 0.

    // 9600
    //out_u32((uint32_t*)UART0_BASE + UART0_IBRD, 19);
    //out_u32((uint32_t*)UART0_BASE + UART0_FBRD, 0);

    // 115200
    out_u32((uint32_t*)(UART0_BASE + UART0_IBRD), 1);
	out_u32((uint32_t*)(UART0_BASE + UART0_FBRD), 40);

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
    out_u32((uint32_t*)(UART0_BASE + UART0_LCRH), /* (1 << 4) |*/ (1 << 5) | (1 << 6));

    // Mask all interrupts.
    out_u32((uint32_t*)(UART0_BASE + UART0_IMSC), (1 << 1) | (1 << 4) | /*(1 << 5) |*/
		    (1 << 6) | (1 << 7) | (1 << 8) |
		    (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    out_u32((uint32_t*)(UART0_BASE + UART0_CR), (1 << 0) | (1 << 8) | (1 << 9));
}

static void bcm2835_uart_putc(const uint32_t const base, uint8_t byte) {
    while (1)
    {
        if (!(in_u32((uint32_t*)(base + UART0_FR)) & (1 << 5)))
        {
        	break;
        }
    }
    out_u32((uint32_t*)(base + UART0_DR), byte);
}

static uint8_t bcm2835_uart_getc(const uint32_t base) {
    while(true)
    {
		if (!(in_u32((uint32_t*)(base + UART0_FR)) & (1 << 4)))
		{
			break;
		}
    }
    return in_u32((uint32_t*)(base + UART0_DR));
}

static error_t bcm2835_uart_isr(
		const void * const usr_data,
		const uint32_t vector)
{
	(void)usr_data;
	(void)vector;
	char buffer[2] = {0, 0};
	buffer[0] = bcm2835_uart_getc((uint32_t)usr_data);
	kernel_in_write(buffer, 1);
	return NO_ERROR;
}

void early_uart_putc(const char c)
{
	bcm2835_uart_putc(UART0_BASE, c);
}

void early_uart_put(const char * c)
{
	if (c)
	{
		while((*c))
		{
			early_uart_putc(*(c++));
		}
	}
}

static error_t bcm2835_uart_write(
		const void * const usr_data,
		const uint32_t id,
		const uint32_t val)
{
	(void)id;
	bcm2835_uart_putc(((usr_data == NULL) ? UART0_BASE : (uint32_t)usr_data), val);
	return NO_ERROR;
}

void bcm2835_uart_get_device(
		kernel_device_t * const device)
{
	if (device)
	{
		device->initialise = NULL;
		device->control = NULL;
		device->read_buffer = NULL;
		device->write_buffer = NULL;
		device->read_register = NULL;
		device->write_register = bcm2835_uart_write;
		uint32_t base = 0;
		kernel_device_map_memory(UART0_BASE, 0x1000, MMU_DEVICE_MEMORY, &base);
		device->user_data = (void*)base;
		device->isr = bcm2835_uart_isr;
	}
}
