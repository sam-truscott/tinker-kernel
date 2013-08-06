/*
 *
 * SOS Source Code
 * __________________
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "print_out.h"
#include "kernel/utils/util_i_to_a.h"
#include "kernel/utils/util_i_to_h.h"
#include "kernel/utils/util_case.h"
#include "arch/board_support.h"
#include <stdarg.h>

#define MAX_INTEGER_LENGTH 10
#define MAX_HEX_INTEGER_LENGTH 8

static void __print_out_process(va_list * const arguments, const char ** const ptr);

static void __print_out_print_char(const char c);

static void __print_out_print_string(const char * string);

static void __print_out_print_signed(const int32_t);

static void __print_out_print_hex(
		const uint32_t,
		const bool upper_case,
		const uint32_t pad);

void __error_print(const char * const msg, ...)
{
	/*__tgt_disable_external_interrupts();*/
	va_list arguments;
	const char * ptr = msg;

	va_start (arguments, msg);

	while(*ptr)
	{
		if(*ptr == '%')
		{
			__print_out_process(&arguments, &ptr);
			ptr++;
		}
		else
		{
			__print_out_print_char(*ptr);
		}
		ptr++;
	}

	va_end(arguments);
	/*__tgt_enable_external_interrupts();*/
}

void __debug_print(const char * const msg, ...)
{
#if defined(__KERNEL_DEBUGGING)
	/*__tgt_disable_external_interrupts();*/
	va_list arguments;
	const char * ptr = msg;

	va_start (arguments, msg);

	while(*ptr)
	{
		if(*ptr == '%')
		{
			__print_out_process(&arguments, &ptr);
			ptr++;
		}
		else
		{
			__print_out_print_char(*ptr);
		}
		ptr++;
	}

	va_end(arguments);
#else
	if (msg) {}
#endif
}

void __print_out_process(va_list * const arguments, const char ** const ptr)
{
	char * rptr = (char*)(*ptr);
	rptr++;
	switch(*rptr)
	{
		case 's':
			{
				const char * const str = (const char *)va_arg(*arguments, char*);
				__print_out_print_string(str);
			}
			break;
		case 'd':
			__print_out_print_signed(va_arg(*arguments, int32_t));
			break;
		case 'x':
			/** TODO work out pad value */
			__print_out_print_hex(va_arg(*arguments, uint32_t), false, 0);
			break;
		case 'X':
			/** TODO work out pad value */
			__print_out_print_hex(va_arg(*arguments, uint32_t), true, 0);
			break;
		default:
			__print_out_print_char('%');
			__print_out_print_char(*rptr);
			break;
	}
}

void __print_out_print_char(const char c)
{
	__bsp_write_debug_char(c);
}

void __print_out_print_string(const char * string)
{
	while(*string)
	{
		__print_out_print_char(*string++);
	}
}

void __print_out_print_signed(const int32_t i)
{
	char number[MAX_INTEGER_LENGTH + 1] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
	__util_i_to_a(i, number, MAX_INTEGER_LENGTH);
	__print_out_print_string(number);
}

void __print_out_print_hex(
		const uint32_t i,
		const bool upper_case,
		const uint32_t pad)
{
	char hex_number[MAX_HEX_INTEGER_LENGTH + 1] = {0,0,0,0,0,0,0,0,0};
	__util_i_to_h(i, hex_number,MAX_HEX_INTEGER_LENGTH);

	if ( upper_case )
	{
		__util_to_upper(hex_number);
	}
	else
	{
		__util_to_lower(hex_number);
	}

	if ( pad > 0 )
	{
		/*
		 * TODO pad the hex strings out
		 */
	}

	__print_out_print_string(hex_number);
}
