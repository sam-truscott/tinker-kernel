
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

#include "tgt_types.h"
#include "tgt_io.h"
#include "bcm2835_uart.h"
#include "utils/util_memset.h"

#pragma GCC diagnostic ignored "-Wcast-align"

// The offsets for reach register.
// Controls actuation of pull up/down to ALL GPIO pins.
#define GPPUD  (0x94)

// Controls actuation of pull up/down for specific GPIO pin.
#define GPPUDCLK0  (0x98)

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

typedef struct bcm2835_user_data
{
	uint32_t base;
	object_pipe_t * input_pipe;
} bcm2835_user_data_t;

/*
 * delay function
 * int32_t delay: number of cycles to delay
 *
 * This just loops <delay> times in a way that the compiler
 * wont optimize away.
 */
static inline void delay(const uint32_t count)
{
    asm volatile(
    		"__delay_%=:"
    		"subs %[count],"
    		"%[count], #1; bne __delay_%=\n"
	     : : [count]"r"(count) : "cc");
}

static bool_t early_available = false;
static uint8_t * early_base = NULL;

static void bcm2835_uart_putc(const uint32_t base, uint8_t byte)
{
	while (1)
	{
		if (!(in_u32((uint32_t*)(base + UART0_FR)) & (1 << 5)))
		{
			break;
		}
	}
	out_u32((uint32_t*)(base + UART0_DR), byte);
}

/*
 * Initialize UART0.
 */
void early_uart_init(const uint8_t * const base_address)
{
    // Disable UART0.
    out_u32((uint32_t*)(base_address + UART0_CR), 0x00000000);
    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    uint8_t * const gpio_base = (uint8_t*)(base_address - 0x1000);
    out_u32((uint32_t*)(gpio_base + GPPUD), 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    out_u32((uint32_t*)(gpio_base + GPPUDCLK0), (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    out_u32((uint32_t*)(gpio_base + GPPUDCLK0), 0x00000000);

    // Clear pending interrupts.
    out_u32((uint32_t*)(base_address + UART0_ICR), 0x7FF);

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	uint32_t new_lcrh = (0 << 4) // disable fifo
    	    		| (1 << 5) | (1 << 6); // 8bit word length
	uint32_t lcrh = in_u32((uint32_t*)(base_address + UART0_LCRH));

	if (new_lcrh != lcrh)
	{
		out_u32((uint32_t*)(base_address + UART0_LCRH), new_lcrh);
	}

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000/(16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    // Divider = 3000000/(16 * 9600) = 19
    // Fractional part register = 0.

    // 9600
    //out_u32((uint32_t*)(base_address + UART0_IBRD), 19);
    //out_u32((uint32_t*)(base_address + UART0_FBRD), 0);

    // 115200
    out_u32((uint32_t*)(base_address + UART0_IBRD), 1);
	out_u32((uint32_t*)(base_address + UART0_FBRD), 40);

	// set the fifo size to smallest
	out_u32((uint32_t*)(base_address + UART0_IFLS), 0);

    // Mask *in* the RX interrupt
    out_u32((uint32_t*)(base_address + UART0_IMSC), (1 << 4));

    // Enable UART0, receive & transfer part of UART.
    out_u32((uint32_t*)(base_address + UART0_CR), (1 << 0) | (1 << 8) | (1 << 9));

    early_base = (uint8_t *)base_address;
	early_available = true;
}

static uint8_t bcm2835_uart_getc(const uint32_t base)
{
    while(true)
    {
		if (!(in_u32((uint32_t*)(base + UART0_FR)) & (1 << 4)))
		{
			break;
		}
    }
    return in_u32((uint32_t*)(base + UART0_DR));
}

static return_t bcm2835_uart_isr(
		const void * const usr_data,
		const uint32_t vector)
{
	// TODO this is only reading one char at a time
	return_t ret = NO_ERROR;
	(void)vector;
	bcm2835_user_data_t * const user_data = (bcm2835_user_data_t*)usr_data;
	if (user_data && user_data->base)
	{
		char buffer[2] = {0, 0};
		buffer[0] = bcm2835_uart_getc(user_data->base);
		if (user_data->input_pipe)
		{
			ret =  kernel_isr_write_pipe(user_data->input_pipe, buffer, 1);
		}
		else
		{
			ret = DEVICE_REGISTER_INVALID;
		}
	}
	else
	{
		ret = DEVICE_REGISTER_INVALID;
	}

	return ret;
}

void early_uart_putc(const char c)
{
	if (early_available)
	{
		bcm2835_uart_putc((uint32_t)(early_base), c);
	}
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

static return_t bcm2835_uart_write(
		const void * const usr_data,
		const uint32_t id,
		const uint32_t val)
{
	(void)id;
	bcm2835_user_data_t * const user_data = (bcm2835_user_data_t*)usr_data;
	if (usr_data == NULL)
	{
		bcm2835_uart_putc((uint32_t)(early_base), val);
	}
	else
	{
		bcm2835_uart_putc(user_data->base, val);
	}
	return NO_ERROR;
}

void bcm2835_uart_get_device(
		const uint8_t * const base_address,
		kernel_device_t * const device,
		const char * const name)
{
	if (device)
	{
		early_available = false;
		device->initialise = NULL;
		device->control = NULL;
		device->read_buffer = NULL;
		device->write_buffer = NULL;
		device->read_register = NULL;
		device->write_register = bcm2835_uart_write;
		uint32_t base = 0;
		kernel_device_map_memory((mem_t)(base_address), 0x1000, MMU_DEVICE_MEMORY, &base);
		device->user_data = kernel_device_malloc(sizeof(bcm2835_user_data_t));
		util_memset(device->user_data, 0, sizeof(bcm2835_user_data_t));
		((bcm2835_user_data_t*)device->user_data)->base = base;
		((bcm2835_user_data_t*)device->user_data)->input_pipe = kernel_isr_get_pipe(name);
		device->isr = bcm2835_uart_isr;
	}
}
