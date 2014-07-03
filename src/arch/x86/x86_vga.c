/*
 *
 * TINKER Source Code
 *
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "x86_vga.h"

#include "kernel/utils/util_strlen.h"

/* Hardware text mode color constants. */
enum vga_color
{
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

static const uint32_t VGA_WIDTH = 80;
static const uint32_t VGA_HEIGHT = 25;

uint32_t terminal_row;
uint32_t terminal_column;
uint8_t terminal_color;
uint16_t * const terminal_buffer = (uint16_t*) 0xB8000;

static uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

void x86_vga_initialise(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
	for ( uint32_t y = 0; y < VGA_HEIGHT; y++ )
	{
		for ( uint32_t x = 0; x < VGA_WIDTH; x++ )
		{
			const uint32_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
	}
}

static void x86_vga_putentryat(char c, uint8_t color, uint32_t x, uint32_t y)
{
	const uint32_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

void x86_vga_putchar(char c)
{
	if (c == '\r')
	{
		terminal_column = 0;
	}
	else if (c == '\n')
	{
		terminal_column = 0;
		if ( (terminal_row + 1) == VGA_HEIGHT )
		{
			const uint16_t* start = terminal_buffer + VGA_WIDTH;
			uint16_t* end = terminal_buffer + (VGA_WIDTH * (VGA_HEIGHT - 1));
			const uint32_t size = end - terminal_buffer;
			//TODO memcpy(terminal_buffer, start, size);
			for (uint16_t s = 0; s < size ; s++)
			{
				terminal_buffer[s] = start[s];
			}
			//TODO memset(end, 0, VGA_WIDTH * sizeof(uint16_t));
			for (uint16_t s = 0 ; s < VGA_WIDTH; s++)
			{
				end[s] = make_vgaentry(' ', terminal_color);
			}
		}
		else
		{
			terminal_row++;
		}
	}
	else
	{
		x86_vga_putentryat(c, terminal_color, terminal_column, terminal_row);
		if ( ++terminal_column == VGA_WIDTH )
		{
			terminal_column = 0;
			if ( ++terminal_row == VGA_HEIGHT )
			{
				terminal_row = 0;
			}
		}
	}
}

void x86_vga_writestring(const char* const data, const uint32_t max)
{
	const uint32_t datalen = util_strlen(data, max);
	for ( uint32_t i = 0; i < datalen; i++ )
		x86_vga_putchar(data[i]);
}


