/*
 *
 * TINKER Source Code
 * 
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

#define MAX_INTEGER_LENGTH 10
#define MAX_HEX_INTEGER_LENGTH 8

static void print_out_process(const char ** const ptr, __builtin_va_list * const list);

static void print_out_print_char(const char c);

static void print_out_print_string(const char * string);

static void print_out_print_signed(const int32_t);

static void print_out_print_hex(
		const uint32_t,
		const bool_t upper_case);

void print_time(void)
{
    tinker_time_t now = TINKER_ZERO_TIME;
	time_get_system_time(&now);
	if (!tinker_time_eq(&now, &TINKER_ZERO_TIME))
	{
		char msg[20];
		util_memset(msg, 0, 20);
		util_i_to_a(now.seconds, msg, 20);
		print_out(msg);
		util_memset(msg, 0, 20);
		util_i_to_a(now.nanoseconds, msg, 20);
		print_out(".");
		print_out(msg);
		print_out(": ");
	}
}

void error_print(const char * const msg, ...)
{
	const char * ptr = msg;

	print_time();

	__builtin_va_list list;
	__builtin_va_start(list, msg);
	while(*ptr)
	{
		if(*ptr == '%')
		{
			print_out_process(&ptr, &list);
			ptr++;
		}
		else
		{
			print_out_print_char(*ptr);
		}
		ptr++;
	}
	__builtin_va_end(list);
}

void debug_print(const char * const msg, ...)
{
#if defined(KERNEL_DEBUGGING)
	const char * ptr = msg;

	print_time();

	__builtin_va_list list;
	__builtin_va_start(list, msg);
	while(*ptr)
	{
		if(*ptr == '%')
		{
			print_out_process(&ptr, &list);
			ptr++;
		}
		else
		{
			print_out_print_char(*ptr);
		}
		ptr++;
	}
	__builtin_va_end(list);
#else
	if (msg) {}
#endif
}

void printp_out(const char * const msg, ...)
{
	const char * ptr = msg;

	__builtin_va_list list;
	__builtin_va_start(list, msg);
	while(*ptr)
	{
		if(*ptr == '%')
		{
			print_out_process(&ptr, &list);
			ptr++;
		}
		else
		{
			print_out_print_char(*ptr);
		}
		ptr++;
	}
	__builtin_va_end(list);
}

void print_out(const char * const msg)
{
	print_out_print_string(msg);
}

void print_out_process(const char ** const ptr, __builtin_va_list * const list)
{
	char * rptr = (char*)(*ptr);
	rptr++;
	switch(*rptr)
	{
		case 's':
			{
				const char * const str = (const char *)__builtin_va_arg(*list, char*);
				print_out_print_string(str);
			}
			break;
		case 'd':
			print_out_print_signed(__builtin_va_arg(*list, int));
			break;
		case 'x':
			print_out_print_hex(__builtin_va_arg(*list, unsigned int), false);
			break;
		case 'X':
			print_out_print_hex(__builtin_va_arg(*list, unsigned int), true);
			break;
		default:
			print_out_print_char('%');
			print_out_print_char(*rptr);
			break;
	}
}

void print_out_print_char(const char c)
{
	bsp_write_debug_char(c);
}

void print_out_print_string(const char * string)
{
	while(*string)
	{
		print_out_print_char(*string++);
	}
}

void print_out_print_signed(const int32_t i)
{
	char number[MAX_INTEGER_LENGTH + 1] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
	util_i_to_a(i, number, MAX_INTEGER_LENGTH);
	print_out_print_string(number);
}

void print_out_print_hex(
		const uint32_t i,
		const bool_t upper_case)
{
	char hex_number[MAX_HEX_INTEGER_LENGTH + 1] = {0,0,0,0,0,0,0,0,0};
	util_i_to_h(i, hex_number,MAX_HEX_INTEGER_LENGTH);

	if (upper_case)
	{
		util_to_upper(hex_number);
	}
	else
	{
		util_to_lower(hex_number);
	}

	print_out_print_string(hex_number);
}
