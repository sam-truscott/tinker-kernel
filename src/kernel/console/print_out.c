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
#include "kernel/utils/util_case.h"
#include "kernel/utils/util_memset.h"
#include "arch/board_support.h"
#include "kernel/time/time_manager.h"
#include <stdarg.h>

#define MAX_INTEGER_LENGTH 10
#define MAX_HEX_INTEGER_LENGTH 8

static void __print_out_process(va_list * const arguments, const char ** const ptr);

static void __print_out_print_char(const char c);

static void __print_out_print_string(const char * string);

static void __print_out_print_signed(const int32_t);

static void __print_out_print_hex(
		const uint32_t,
		const bool_t upper_case);

void __print_time(void)
{
	sos_time_t now = SOS_ZERO_TIME;
	__time_get_system_time(&now);
	char msg[20];
	memset(msg, 0, 20);
	__util_i_to_a(now.seconds, msg, 20);
	__print_out(msg);
	memset(msg, 0, 20);
	__util_i_to_a(now.nanoseconds, msg, 20);
	__print_out(".");
	__print_out(msg);
	__print_out(": ");
}

void __error_print(const char * const msg, ...)
{
	/*__tgt_disable_external_interrupts();*/
	va_list arguments;
	const char * ptr = msg;

	__print_time();

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

	__print_time();

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

void __printp_out(const char * const msg, ...)
{
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
}

void __print_out(const char * const msg)
{
	__print_out_print_string(msg);
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
			__print_out_print_hex(va_arg(*arguments, uint32_t), false);
			break;
		case 'X':
			/** TODO work out pad value */
			__print_out_print_hex(va_arg(*arguments, uint32_t), true);
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
		const bool_t upper_case)
{
	char hex_number[MAX_HEX_INTEGER_LENGTH + 1] = {0,0,0,0,0,0,0,0,0};
	__util_i_to_h(i, hex_number,MAX_HEX_INTEGER_LENGTH);

	if (upper_case)
	{
		__util_to_upper(hex_number);
	}
	else
	{
		__util_to_lower(hex_number);
	}

	__print_out_print_string(hex_number);
}
