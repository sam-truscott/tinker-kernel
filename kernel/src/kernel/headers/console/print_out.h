/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#ifndef PRINT_OUT_H_
#define PRINT_OUT_H_

#include "time/time_manager.h"

void print_set_time_manager(time_manager_t * const tm);

void print_time(void);

void error_print(const char * const msg, ...);

void debug_print(const char * const msg, ...);

void printp_out(const char * const msg, ...);

void print_out(const char * const msg);

#endif /* DEBUG_PRINT_H_ */
