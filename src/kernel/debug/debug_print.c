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

#include "debug_print.h"
#include "../utils/util_i_to_a.h"
#include "../utils/util_i_to_h.h"
#include "../utils/util_case.h"

#ifndef __KERNEL_DEBUGGING

void __debug_print(const char * msg, ...)
{
	/* Don't do anything */
	if ( msg ) {}
}

#else

#include "../../arch/board_support.h"

#include <stdarg.h>

#define MAX_INTEGER_LENGTH 10
#define MAX_HEX_INTEGER_LENGTH 8

static void __debug_process_lookup(va_list * arguments, char ** ptr);

static void __debug_print_char(const char c);

static void __debug_print_string(const char * string);

static void __debug_print_signed(const int32_t);

static void __debug_print_hex(const uint32_t, bool upper_case, uint32_t pad);

void __debug_print(const char * msg, ...)
{
	/*__tgt_disable_external_interrupts();*/
	va_list arguments;
	char * ptr = (char*)msg;

	va_start (arguments, msg);

	while(*ptr)
	{
		if(*ptr == '%')
		{
			__debug_process_lookup(&arguments, &ptr);
			ptr++;
		}
		else
		{
			__debug_print_char(*ptr);
		}
		ptr++;
	}

	va_end(arguments);
	/*__tgt_enable_external_interrupts();*/
}

void __debug_process_lookup(va_list * arguments, char ** ptr)
{
	char * rptr = *ptr;
	rptr++;
	switch(*rptr)
	{
		case 's':
			{
				const char * str = (const char *)va_arg(*arguments, char*);
				__debug_print_string(str);
			}
			break;
		case 'd':
			__debug_print_signed(va_arg(*arguments, int32_t));
			break;
		case 'x':
			/** TODO work out pad value */
			__debug_print_hex(va_arg(*arguments, uint32_t), false, 0);
			break;
		case 'X':
			/** TODO work out pad value */
			__debug_print_hex(va_arg(*arguments, uint32_t), true, 0);
			break;
		default:
			__debug_print_char('%');
			__debug_print_char(*rptr);
			break;
	}
}

void __debug_print_char(const char c)
{
	__bsp_write_debug_char(c);
}

void __debug_print_string(const char * string)
{
	while(*string)
	{
		__debug_print_char(*string++);
	}
}

void __debug_print_signed(const int32_t i)
{
	char number[MAX_INTEGER_LENGTH + 1] = {'\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
	__util_i_to_a(i, number, MAX_INTEGER_LENGTH);
	__debug_print_string(number);
}

void __debug_print_hex(const uint32_t i, bool upper_case, uint32_t pad)
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

	__debug_print_string(hex_number);
}

#endif
