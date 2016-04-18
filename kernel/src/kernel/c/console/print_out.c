/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "console/print_out.h"
#include "utils/util_i_to_a.h"
#include "utils/util_case.h"
#include "utils/util_memset.h"
#include "utils/util_strlen.h"
#include "utils/util_a_to_i.h"
#include "board_support.h"
#include "time/time_manager.h"

#define MAX_INTEGER_LENGTH 10
#define MAX_HEX_INTEGER_LENGTH 8

static time_manager_t * time_manager = NULL;

static void print_out_process(const char ** const ptr, __builtin_va_list * const list);

static void print_out_print_char(const char c);

static void print_out_print_string(const uint32_t padding, const char * string);

static void print_out_print_signed(const uint32_t padding, const int32_t);

static void print_out_print_hex(
		const uint32_t padding,
		const uint32_t,
		const bool_t upper_case);

void print_set_time_manager(time_manager_t * const tm)
{
	time_manager = tm;
}

void print_time(void)
{
    tinker_time_t now = TINKER_ZERO_TIME;
	time_get_system_time(time_manager, &now);
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

void debug_print1(
		const bool log,
		const char * const file,
		const uint32_t line,
		const char * const msg,
		...)
{
#if defined(KERNEL_DEBUGGING)
	if (log)
	{
		const char * ptr = msg;

		print_time();
		print_out("F:");
		print_out(file);
		printp_out(" L:%d ", line);

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
#else
	(void)log;
	(void)msg;
	(void)file;
	(void)line;
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
	print_out_print_string(0, msg);
}

#define MAX_PADDING_CHARS 9

void print_out_process(const char ** const ptr, __builtin_va_list * const list)
{
	char pad[MAX_PADDING_CHARS];
	util_memset(pad, 0, MAX_PADDING_CHARS);
	unsigned char padp = 0;
	char * rptr = (char*)(*ptr);
start:
	rptr++;
	switch(*rptr)
	{
		case 's':
			{
				const char * const str = (const char *)__builtin_va_arg(*list, char*);
				print_out_print_string(util_a_to_i(pad, padp), str);
			}
			break;
		case 'd':
			print_out_print_signed(util_a_to_i(pad, padp), __builtin_va_arg(*list, int));
			break;
		case 'x':
			print_out_print_hex(util_a_to_i(pad, padp), __builtin_va_arg(*list, unsigned int), false);
			break;
		case 'X':
			print_out_print_hex(util_a_to_i(pad, padp), __builtin_va_arg(*list, unsigned int), true);
			break;
		default:
			pad[padp++] = *(rptr);
			(*ptr)++;
			goto start;
	}
}

void print_out_print_char(const char c)
{
	bsp_write_debug_char(c);
}

void print_out_print_string(const uint32_t padding, const char * string)
{
	uint32_t size;
	if ((size=util_strlen(string, MAX_PADDING)) < padding)
	{
		for (uint32_t i = 0 ; i < padding-size ; i++)
		{
			print_out_print_char('0');
		}
	}
	while (*string)
	{
		print_out_print_char(*string++);
	}
}

void print_out_print_signed(const uint32_t padding, const int32_t i)
{
	char number[MAX_INTEGER_LENGTH + 1] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
	util_i_to_a(i, number, MAX_INTEGER_LENGTH);
	print_out_print_string(padding, number);
}

void print_out_print_hex(
		const uint32_t padding,
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

	print_out_print_string(padding, hex_number);
}
